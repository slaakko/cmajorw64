        EXTERN strlen,puts,putsf,putnl,putnlf,newline

.CODE
        
strlen  FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,1*8
        SET $0,ax
        SET ax,0
@1      LDB $1,$0,0
        BZ $1,@2
        ADD ax,ax,1
        ADD $0,$0,1
        JMP @1
@2      SET sp,fp
        LDO fp,sp,0
        RET
strlen  ENDF

puts    FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,1*8
        SET bx,1
        CALL $0,putsf
        SET sp,fp
        LDO fp,sp,0
        RET
puts    ENDF

putsf   FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,1*8
        SET $1,bx
        SET $0,ax
        CALL $2,strlen
        SET cx,ax
        SET bx,$0
        SET ax,$1
        CALL $2,write
        SET sp,fp
        LDO fp,sp,0
        RET
putsf   ENDF

putnl   FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,1*8
        SET ax,1
        CALL $0,putnlf
        SET sp,fp
        LDO fp,sp,0
        RET
putnl   ENDF

putnlf  FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,1*8
        SET bx,ax
        LDOU ax,newline
        CALL $0,putsf
        SET sp,fp
        LDO fp,sp,0
        RET
putnlf  ENDF

.DATA

newline OCTA nl
nl      BYTE 10,0
