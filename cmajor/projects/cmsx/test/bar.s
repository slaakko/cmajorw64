        EXTERN main
        LINKONCE foo,bar

.CODE

main    FUNC
        GO $0,foo
        GO $0,bar
        GO $0,baz
main    ENDF

foo     FUNC
        GO $0,$0
foo     ENDF

bar     FUNC
        GO $0,$0
bar     ENDF
