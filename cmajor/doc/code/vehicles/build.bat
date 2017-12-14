@echo off
cmc --emit-llvm vehicles.cmp
cm2html --style ../../style/code.css vehicles.cm
