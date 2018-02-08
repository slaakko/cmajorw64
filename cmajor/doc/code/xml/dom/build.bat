@echo off
cmc --config=release dom.cmp
cm2html --style ../../../style/code.css dom.cm
