@echo off
cmc --config=release xmlstats.cmp
cm2html --style ../../../style/code.css xmlstats.cm
