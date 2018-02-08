@echo off
cmc --config=release deflate.cmp
cm2html --style ../../../style/code.css main.cm
