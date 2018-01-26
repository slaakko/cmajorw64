@echo off
set PATH=C:\mingw-w64-5.4.0\mingw64\bin;%PATH%
gcc -c -O2 zlib_intf.c -o zlib_intf.o
gcc -shared zlib_intf.o libz.a -o cmrt200zlib.dll
dlltool --verbose --output-def cmrt200zlib.def zlib_intf.o
lib /MACHINE:X64 /DEF:cmrt200zlib.def /OUT:cmrt200zlib.lib
