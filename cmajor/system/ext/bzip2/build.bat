@echo off
set PATH=C:\mingw-w64-5.4.0\mingw64\bin;%PATH%
gcc -c -O2 bz2_intf.c -o bz2_intf.o
gcc -shared bz2_intf.o libbz2.a -o cmrt200bz2.dll
dlltool --verbose --output-def cmrt200bz2.def bz2_intf.o
lib /MACHINE:X64 /DEF:cmrt200bz2.def /OUT:cmrt200bz2.lib
