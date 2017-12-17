@echo off
cmc --emit-llvm interface.cmp
cm2html --style ../../style/code.css interface.cm
