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
