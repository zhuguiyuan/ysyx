#!/bin/env bash

for i in {0,1,2}
do
    echo === bench -O${i} ===
    clang -O${i} sum.c -o sum
    llvm-objdump -d sum > sum_O${i}.txt
    time ./sum 10000000000
    rm -f sum
done
