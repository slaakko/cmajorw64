            EXTERN exit,wait,fork,write

sys_exit    IS 0
sys_wait    IS 1
sys_fork    IS 2
sys_write   IS 3

exit        FUNC
            TRAP 0,sys_exit,0
exit        ENDF

wait        FUNC
            TRAP 0,sys_wait,0
            GO $0,$0,0
wait        ENDF

fork        FUNC
            TRAP 0,sys_fork,0
            GO $0,$0,0
fork        ENDF

write       FUNC
            TRAP 0,sys_write,0
            GO $0,$0,0
write       ENDF

template    FUNC
            STO fp,sp,0
            SET fp,sp
            INCL sp,2*8
            STO $0,fp,8
            LDO $0,fp,8
            SET sp,fp
            LDO fp,sp,0
template    ENDF
