@echo off

Dua -S -emit-llvm -no-libdua common.c
Dua -S -emit-llvm -no-libdua algorithms.dua
Dua -S -emit-llvm -no-libdua c.dua
Dua -S -emit-llvm -no-libdua execution.dua
Dua -S -emit-llvm -no-libdua globals.dua
Dua -S -emit-llvm -no-libdua io.dua
Dua -S -emit-llvm -no-libdua priority-queue.dua
Dua -S -emit-llvm -no-libdua random.dua
Dua -S -emit-llvm -no-libdua string.dua
Dua -S -emit-llvm -no-libdua vector.dua

clang -c common.ll
clang -c algorithms.ll
clang -c c.ll
clang -c execution.ll
clang -c globals.ll
clang -c io.ll
clang -c priority-queue.ll
clang -c random.ll
clang -c string.ll
clang -c vector.ll

ar rcs dua.lib *.o

del common.ll
del algorithms.ll
del c.ll
del execution.ll
del globals.ll
del io.ll
del priority-queue.ll
del random.ll
del string.ll
del vector.ll

del common.o
del algorithms.o
del c.o
del execution.o
del globals.o
del io.o
del priority-queue.o
del random.o
del string.o
del vector.o