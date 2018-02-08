@echo off
cmc --config=release xpath.cmp
cm2html --style ../../../style/code.css xpath.cm
