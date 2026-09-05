ALL += a.o a.txt a.rv64.o a.rv64.txt a.ll.txt a.rv64.ll.txt a.out.log.txt a.rv64.out.txt

all: $(ALL)

a.o: a.c
	clang -c a.c

a.out.log.txt: a.c
	clang a.c --verbose 2> a.out.log.txt
	rm -f a.out

a.rv64.o: a.c
	clang -c a.c --target=riscv64-linux-gnu -o a.rv64.o

a.txt: a.o
	objdump -d a.o > a.txt

a.ll.txt: a.o
	llvm-objdump -d a.o > a.ll.txt

a.rv64.txt: a.rv64.o
	riscv64-linux-gnu-objdump -d a.rv64.o > a.rv64.txt

a.rv64.ll.txt: a.rv64.o
	llvm-objdump -d a.rv64.o > a.rv64.ll.txt

a.rv64.out.txt: a.c
	clang a.c --target=riscv64-linux-gnu -o a.rv64.out --verbose
	llvm-objdump -d a.rv64.out > a.rv64.out.txt
	rm -f a.rv64.out

clean:
	rm -f $(ALL)

.PHONY: all clean
