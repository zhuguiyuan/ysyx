# 二进制文件的生成和执行

## 查看 riscv64 目标文件的反汇编结果

相比于汇编的结果，反汇编的结果少了很多 attribute 标注信息和编译相关的指示信息。指令上也有一些变化，比如汇编出来会有伪指令 `call`，反汇编之后变成了 `jalr`。

## 查看 riscv64 可执行文件的反汇编结果

可执行文件的反汇编结果相比于链接前的多了一些来自其他目标文件的段，比如 `_start` 等。但是主体的 `main` 部分相比目标文件反汇编的结果还是差不多的。

## 对比编译优化前后的而性能差异

非常 impressive。

```bash
=== bench -O0 ===
sum(1, 10000000000) = -5340232216128654848

real    0m6.210s
user    0m6.210s
sys     0m0.000s
=== bench -O1 ===
sum(1, 10000000000) = -5340232216128654848

real    0m0.001s
user    0m0.001s
sys     0m0.000s
=== bench -O2 ===
sum(1, 10000000000) = -5340232216128654848

real    0m0.001s
user    0m0.000s
sys     0m0.001s
```

根据反汇编的结果，-O0 每次都老老实实做了循环；-O1 和 -O2 生成了同样的二进制，直接用高斯公式来求和了。这触发了 LLVM 的 ScalarEvolution。

## 程序真的从 `main()` 开始执行吗？

用 strace 分析

```bash
$ strace ./sum 10
execve("./sum", ["./sum", "10"], 0x7ffff46284c8 /* 111 vars */) = 0
brk(NULL)                               = 0x557a167bc000
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f654b668000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (没有那个文件或目录)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=137803, ...}) = 0
mmap(NULL, 137803, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f654b646000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0p\236\2\0\0\0\0\0"..., 832) = 832
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 840, 64) = 840
fstat(3, {st_mode=S_IFREG|0755, st_size=1995216, ...}) = 0
pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 840, 64) = 840
mmap(NULL, 2047568, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f654b452000
mmap(0x7f654b47a000, 1454080, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x28000) = 0x7f654b47a000
mmap(0x7f654b5dd000, 352256, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x18b000) = 0x7f654b5dd000
mmap(0x7f654b633000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e0000) = 0x7f654b633000
mmap(0x7f654b639000, 52816, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f654b639000
close(3)                                = 0
mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f654b44f000
arch_prctl(ARCH_SET_FS, 0x7f654b44f740) = 0
set_tid_address(0x7f654b44fa10)         = 181263
set_robust_list(0x7f654b44fa20, 24)     = 0
rseq(0x7f654b44f680, 0x20, 0, 0x53053053) = 0
mprotect(0x7f654b633000, 16384, PROT_READ) = 0
mprotect(0x557a0f282000, 4096, PROT_READ) = 0
mprotect(0x7f654b6a4000, 8192, PROT_READ) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
munmap(0x7f654b646000, 137803)          = 0
fstat(1, {st_mode=S_IFCHR|0600, st_rdev=makedev(0x88, 0x2), ...}) = 0
getrandom("\x2d\xe4\xaf\xc4\x6a\x57\xf6\x10", 8, GRND_NONBLOCK) = 8
brk(NULL)                               = 0x557a167bc000
brk(0x557a167dd000)                     = 0x557a167dd000
write(1, "sum(1, 10) = 55\n", 16sum(1, 10) = 55
)       = 16
exit_group(0)                           = ?
+++ exited with 0 +++
```

利用 AI 的帮助实现了一个 gbd 的脚本来帮助分析，见 `analyze_trace.py`。
