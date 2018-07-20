cmc --ast2xml --sym2xml --bdt2xml --emit-llvm QuickSort.cmp
copy lib/debug/QuickSort.ll .
cm2html --style ../../style/code.css QuickSort.cm
