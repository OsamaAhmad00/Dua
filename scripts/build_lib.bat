@echo off

"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/common.c
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/algorithms.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/c.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/execution.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/globals.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/io.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/priority-queue.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/random.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/string.dua
"%ProgramFiles%\Dua\Dua.exe" -S -emit-llvm -no-libdua ../lib/vector.dua

"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c common.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c algorithms.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c c.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c execution.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c globals.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c io.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c priority-queue.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c random.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c string.ll
"%ProgramFiles%\Dua\Dua.exe" -no-libdua -c vector.ll

llvm-ar rcs dua.lib *.o

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