        EXTERN foo,ext

.CODE

foo     FUNC
        GO $0,Exit
foo     ENDF        

ext     FUNC
        GO $0,Exit
ext     ENDF
