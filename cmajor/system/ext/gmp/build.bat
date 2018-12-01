@echo off
set PATH=C:\mingw-w64-8.1.0\mingw64\bin;%PATH%
gcc -c -O2 gmpintf.c -o gmpintf.o
gcc -shared gmpintf.o libgmp.a -o cmrt300gmp.dll
dlltool --verbose --output-def cmrt300gmp.def gmpintf.o
lib /MACHINE:X64 /DEF:cmrt300gmp.def /OUT:cmrt300gmp.lib
