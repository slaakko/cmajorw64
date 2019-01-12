        EXTERN main,sub,rec

.CODE

main    FUNC
        LDB $0,data
        STB $0,data
        GO $0,sub
        BZ ax,@1
        TRAP 0,0,0
@1      LDB $0,data
        BNZ $0,@1
        TRAP 0,0,0
        JMP sub
        JMP foo
main    ENDF        
        
sub     FUNC
        GO $0,$0
sub     ENDF        

.DATA

data    BYTE 0
rec     STRUCT
        OCTA ext
rec     ENDS        
