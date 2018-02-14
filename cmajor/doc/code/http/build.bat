@echo off
cmc --config=release httpClient.cmp
cm2html --style ../../style/code.css httpClient.cm
