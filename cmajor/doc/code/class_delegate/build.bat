@echo off
cmc --emit-llvm class_delegate.cmp
cm2html --style ../../style/code.css class_delegate.cm
