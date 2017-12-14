@echo off
cmc --emit-llvm console.cmp
cm2html --style ../../style/code.css console.cm
