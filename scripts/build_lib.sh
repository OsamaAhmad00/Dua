Dua -S -emit-llvm -no-libdua ../lib/common.c
Dua -S -emit-llvm -no-libdua ../lib/algorithms.dua
Dua -S -emit-llvm -no-libdua ../lib/c.dua
Dua -S -emit-llvm -no-libdua ../lib/execution.dua
Dua -S -emit-llvm -no-libdua ../lib/globals.dua
Dua -S -emit-llvm -no-libdua ../lib/io.dua
Dua -S -emit-llvm -no-libdua ../lib/priority-queue.dua
Dua -S -emit-llvm -no-libdua ../lib/random.dua
Dua -S -emit-llvm -no-libdua ../lib/string.dua
Dua -S -emit-llvm -no-libdua ../lib/vector.dua

Dua -c -no-libdua common.ll
Dua -c -no-libdua algorithms.ll
Dua -c -no-libdua c.ll
Dua -c -no-libdua execution.ll
Dua -c -no-libdua globals.ll
Dua -c -no-libdua io.ll
Dua -c -no-libdua priority-queue.ll
Dua -c -no-libdua random.ll
Dua -c -no-libdua string.ll
Dua -c -no-libdua vector.ll

ar rcs libdua.a *.o

rm common.ll
rm algorithms.ll
rm c.ll
rm execution.ll
rm globals.ll
rm io.ll
rm priority-queue.ll
rm random.ll
rm string.ll
rm vector.ll

rm common.o
rm algorithms.o
rm c.o
rm execution.o
rm globals.o
rm io.o
rm priority-queue.o
rm random.o
rm string.o
rm vector.o
