@echo off
cmc --emit-llvm unique_ptr.cmp
cm2html --style ../../style/code.css unique_ptr.cm
