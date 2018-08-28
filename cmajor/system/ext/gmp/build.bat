@echo off
set PATH=C:\mingw-w64-5.4.0\mingw64\bin;%PATH%
gcc -c -O2 gmpintf.c -o gmpintf.o
gcc -shared gmpintf.o libgmp.a -o cmrt230gmp.dll
dlltool --verbose --output-def cmrt230gmp.def gmpintf.o
lib /MACHINE:X64 /DEF:cmrt230gmp.def /OUT:cmrt230gmp.lib
