            EXTERN Exit,Wait,Fork

sys_exit    IS 0
sys_wait    IS 1
sys_fork    IS 2

Exit        FUNC
            TRAP 0,sys_exit,0
Exit        ENDF

Wait        FUNC
            STO fp,sp,0
            SET fp,sp
            INCL sp,2*8
            STO $0,fp,8
            TRAP 0,sys_wait,0
            LDO $0,fp,8
            SET sp,fp
            LDO fp,sp,0
            GO $0,$0,0
Wait        ENDF

Fork        FUNC
            STO fp,sp,0
            SET fp,sp
            INCL sp,2*8
            STO $0,fp,8
            TRAP 0,sys_fork,0
            LDO $0,fp,8
            SET sp,fp
            LDO fp,sp,0
            GO $0,$0,0
Fork        ENDF
