        EXTERN baz
        LINKONCE foo,bar

.CODE

foo     FUNC
        GO $0,$0
foo     ENDF

bar     FUNC
        GO $0,$0
bar     ENDF

baz     FUNC
        GO $0,Exit
baz     ENDF
