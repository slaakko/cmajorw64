@echo off
set PATH=C:\mingw-w64-5.4.0\mingw64\bin;%PATH%
gcc -c -O2 gmpintf.c -o gmpintf.o
gcc -shared gmpintf.o libgmp.a -o cmrt210gmp.dll
dlltool --verbose --output-def cmrt210gmp.def gmpintf.o
lib /MACHINE:X64 /DEF:cmrt210gmp.def /OUT:cmrt210gmp.lib
