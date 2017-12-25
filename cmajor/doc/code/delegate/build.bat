@echo off
cmc --emit-llvm delegate.cmp
cm2html --style ../../style/code.css delegate.cm
