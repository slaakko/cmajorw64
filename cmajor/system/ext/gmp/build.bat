@echo off
set PATH=C:\mingw-w64-5.4.0\mingw64\bin;%PATH%
gcc -c -O2 gmpintf.c -o gmpintf.o
gcc -shared gmpintf.o libgmp.a -o cmrt200gmp.dll
dlltool --verbose --output-def cmrt200gmp.def gmpintf.o
lib /MACHINE:X64 /DEF:cmrt200gmp.def /OUT:cmrt200gmp.lib
