# ScalarEvolution 详解

> 背景：见 `sum_bench/`。`-O1`/`-O2` 把 `sum.c` 的 10^10 次循环直接用高斯公式替换掉了，背后的机理是 LLVM 的 ScalarEvolution（SCEV）分析 + IndVarSimplify 优化。

## 1. 它是什么

ScalarEvolution 是 LLVM 中一个**只读的函数级分析 pass**：它不修改 IR，只回答问题——**"某个标量值在第 k 次循环迭代时等于什么？"**

- 名字的来源：循环里的变量值随迭代"演化"（evolve），SCEV 把这种演化规律抽象成符号表达式
- 理论基础：**Chains of Recurrences（链式递推，CR 形式）**，来自 Robert van Engelen 2001 年 CC 会议论文 *Efficient Symbolic Analysis for Optimizing Compilers*，LLVM 完整采纳了这套记号
- 位置：`llvm/include/llvm/Analysis/ScalarEvolution.h`，核心实现约 1.5 万行的 `ScalarEvolution.cpp`

它依赖 LoopInfo、DominatorTree 等前置分析，并且要求循环处于**规范化形式**（有 preheader、单一 backedge、退出块 LCSSA），所以通常只对 LoopSimplify/LoopRotate 之后的 IR 生效。

## 2. 核心抽象：SCEV 表达式层次

所有值被归类为一种 SCEV 表达式：

| 类型 | 记号 | 含义 |
|---|---|---|
| `SCEVUnknown` | `%4` | 分析不了，直接指向原 IR Value |
| `SCEVConstant` | `8` | 常量 |
| **`SCEVAddRecExpr`** | `{1,+,1}<L>` | **加法递推**（主角）：从 1 出发，循环 L 每迭代一次 +1 |
| `SCEVAddExpr` | `(8 + %17)` | 加法 |
| `SCEVMulExpr` | `(2 * %n)` | 乘法 |
| `SCEVSMax/UMaxExpr` | `(1 smax %9)` | 有符号/无符号 max |
| `SCEVUDivExpr` | `x /u 2` | 无符号除法 |
| Cast 族 | `zext i64 ... to i65` | 扩展/截断 |

AddRec 的完整语义：`{base, +, step}<L>` 在第 i 次迭代时等于 `base + step·i`。关键点在于 **step 本身也可以是 AddRec**，嵌套起来就是链式递推，比如 `{0,+,1,+,1}` 是多操作数写法，等价于 `{{0,+,1},+,1}`：每次迭代的增量本身又在按递推增长——这就是二次多项式。此外 AddRec 可以带 `<nuw>/<nsw>` 标记，记录 IR 里 `add` 指令的 no-wrap 信息，供范围分析使用。

## 3. 用 sum.c 走一遍

对规范化后的循环跑 `opt -passes='print<scalar-evolution>'`，实际输出（节选）：

```llvm
%.01 = phi i64 [ 1, %6 ], [ %12, %10 ]        ; i
  -->  {1,+,1}<nuw><nsw><%10>                 ; 每次迭代 +1
%.02 = phi i64 [ 0, %6 ], [ %11, %10 ]        ; sum
  -->  {0,+,1,+,1}<%10>                       ; 二次递推！
%11 = add nuw nsw i64 %.02, %.01              ; sum + i
  -->  {1,+,2,+,1}<%10>
      Exits: (-1 + (trunc i65 (((zext i64 (-2 + (1 smax %9))<nsw> to i65)
             * (zext i64 (-1 + (1 smax %9))<nsw> to i65)) /u 2) to i64)
             + (2 * (1 smax %9))<nuw>)

Determining loop execution counts for: @main
Loop %10: backedge-taken count is (-1 + (1 smax %9))<nsw>
```

几个要点：

**（1）`sum` 被识别为二次递推** `{0,+,1,+,1}`：迭代 i 次后等于 0 + Σ(1 + Σ1) = i(i+1)/2。SCEV 看到了 `sum` 的步长是 `i`，而 `i` 自己又是 `{1,+,1}`，于是嵌套成链式递推。

**（2）闭式求值用二项式系数公式**：`SCEVAddRecExpr::evaluateAtIteration` 对多项式型递推直接套 `{a,+,b,+,c}(n) = a·C(n,0) + b·C(n,1) + c·C(n,2)`。`%11 = {1,+,2,+,1}` 在最后一次迭代（k = n−1）处求值：

```
1·C(n-1,0) + 2·C(n-1,1) + 1·C(n-1,2) = 1 + 2(n-1) + (n-1)(n-2)/2
                                     = 2n - 1 + (n-1)(n-2)/2 = n(n+1)/2
```

**这正是反汇编里那段神秘代码**！逐条对应：

| 汇编 | SCEV 表达式 |
|---|---|
| `cmovgeq %rax, %rcx` | `(1 smax %9)` —— 把 n 钳到 ≥1（因为 SCEV 无法证明参数 ≥1，不钳的话 n−1 可能是负数，公式就错了） |
| `leaq -1(%rcx)` / `leaq -2(%rcx)` | `(-1 + n)`、`(-2 + n)` |
| `mulq %rdx` + `shldq $0x3f` | 64→128 位乘法再右移 1，即 `zext ... to i65` 乘完 `/u 2`——除法精度处理：64 位乘积可能溢出，所以提升到 65 位再除 |
| `leaq (%rdx,%rcx,2)` + `decq` | `(2·n) + … − 1` |

**（3）trip count 也被算出来了**：`backedge-taken count = (-1 + (1 smax %9))<nsw>`，即回边执行 max(til,1)−1 次。

## 4. 谁在消费它：本次案例的完整链路

`-print-before=indvars` 的 dump 揭示了真正的执行者是 **IndVarSimplify**（不是 SCEV 自己）：它调用 `rewriteLoopExitValues`，发现循环外 `printf` 使用的 `%11` 的 SCEV **在循环退出处可计算（LoopDispositions: Computable）**，于是用 **SCEVExpander** 把这个闭式表达式机械地展开成 IR（`%11 = smax` → `%17 = mul i65` 那一串），替换掉循环外的使用；替换后循环成了死代码（`br i1 false`），被后续 pass 删除。

```llvm
; *** IR Dump After IndVarSimplifyPass ***
%11 = call i64 @llvm.smax.i64(i64 %10, i64 1)
%12 = shl nuw i64 %11, 1
%13 = add nsw i64 %11, -1
%14 = zext nneg i64 %13 to i65
%15 = add nsw i64 %11, -2
%16 = zext i64 %15 to i65
%17 = mul i65 %14, %16
%18 = lshr i65 %17, 1
%19 = trunc i65 %18 to i64
%20 = add i64 %12, %19
br label %21
; Loop:
21:                                               ; preds = %21, %7
  br i1 false, label %21, label %22
```

**分工**：SCEV 只负责"算出来"，IndVarSimplify 负责"替换掉"，SCEVExpander 负责"生成代码"。

SCEV 的其他主要消费者：

- **IndVarSimplify**：除退出值替换外，还做归纳变量强度削减（把 `i*8` 降级成每次 `+8`）、消除多余的 IV
- **LoopStrengthReduce（LSR）**：地址计算的强度削减，比如数组访问 `a[i]` 的地址用 `{base,+,8}` 表示
- **循环展开/全展开**：`getSmallConstantTripCount` 得知循环次数是常量 4，直接全展开
- **循环向量化**：分析内存访问的 stride 和依赖距离，判断能否向量化
- **LoopIdiomRecognize**：识别 memset/memcpy/位计数等模式（也是靠 SCEV 描述的模式）
- **运行时检查生成**：如 128 位乘除的 overflow check

## 5. 它是怎么算 trip count 的

对每个退出块，SCEV 把退出条件写成方程（`HowFarToZero` 家族）：比如条件 `%.01 < %9` 对应求解 `{1,+,1} ≥ %9` 的最小迭代次数，解出 `(-1 + (1 smax %9))`。多个退出块时取最小值。计算全程依赖 `<nsw>` 等 flag 和 ConstantRange（输出里的 `U: [1,-9223372036854775808) S: [...]` 就是无符号/有符号值域）做化简与证明。

## 6. 局限与坑

- **只是分析，正确性有前提**：它敢利用 C 的有符号溢出 UB 假设（`<nsw>` 不溢出），一旦程序实际溢出，结果可能不正确——LLVM 历史上出过不少 SCEV 相关 bug
- **load 一律是 SCEVUnknown**（看输出里 `%4 → %4 U: full-set`），它不穿透内存
- **只擅长仿射/多项式**，遇到非规则、依赖数组下标的数据相关性就无能为力
- **维护成本高**：结果大量缓存（ValueExprMap、FoldingSet），循环被改过要 `forgetLoop`/`forgetValue`，忘调用就是臭名昭著的 stale 缓存 bug

## 7. 自己动手看

```bash
# 打印某个函数的 SCEV 分析结果
opt -passes='print<scalar-evolution>' sum_canon.ll -o /dev/null 2>&1
# 看 IndVarSimplify 前后的 IR（本次案例的关键现场）
clang -O1 -mllvm -print-before=indvars -mllvm -print-after=indvars -S -emit-llvm sum.c -o /dev/null
# 打印 SCEV 内部决策过程（需要 asserts 版 LLVM）
opt -debug-only=scalar-evolution ...
```

一个坑：`clang -O0 -S -emit-llvm` 输出的 IR 带 `optnone` 属性，opt 的所有 pass 都会跳过它——构造演示时要用 `clang -O1 -Xclang -disable-llvm-passes` 拿"没优化过但不带 optnone"的 IR。

## 8. 进一步阅读

- Robert van Engelen, *Efficient Symbolic Analysis for Optimizing Compilers*（CR 形式的原始论文）
- [ScalarEvolution.h](https://llvm.org/doxygen/ScalarEvolution_8h.html) 的头注释是极好的导览，每个 SCEV 类型都有详细定义
- 龙书第 10 章"induction variable analysis"——SCEV 可以看作它的现代符号化版本

一句话总结：**SCEV 是把"循环里值的变化规律"抽象成可运算符号表达式的分析框架，sum 基准里 6000 倍的提速，就是 IndVarSimplify 借它算出了 sum 在循环出口的闭式解 n(n+1)/2，再用 SCEVExpander 生成那几行神秘指令把循环整个删掉的结果。**
