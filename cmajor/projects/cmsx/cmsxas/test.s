.CODE
Main    LDB $0,Data
        STB $0,Data
        GO $0,Sub
        BZ ax,Over
        TRAP 0,0,0
Over    LDB $0,Data
        BNZ $0,Over
        TRAP 0,0,0
        JMP Sub
        JMP Foo
Sub     GO $0,$0

.DATA
Data    BYTE 0
        OCTA ext

