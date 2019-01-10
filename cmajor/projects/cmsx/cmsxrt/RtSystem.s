sys_exit    IS 0
sys_wait    IS 1
sys_fork    IS 2

Exit    TRAP 0,sys_exit,0

Wait    STO fp,sp,0
        SET fp,sp
        INCL sp,1*8
        STO $0,fp,8
        TRAP 0,sys_wait,0
        LDO $0,fp,8
        SET sp,fp
        LDO fp,sp,0
        GO $0,$0,0

Fork    STO fp,sp,0
        SET fp,sp
        INCL sp,1*8
        STO $0,fp,8
        TRAP 0,sys_fork,0
        LDO $0,fp,8
        SET sp,fp
        LDO fp,sp,0
        GO $0,$0,0
