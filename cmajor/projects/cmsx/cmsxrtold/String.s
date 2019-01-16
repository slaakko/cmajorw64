        EXTERN strlen,puts,putsf,putnl,putnlf,newline

.CODE
        
strlen  FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,5*8
        STO $0,fp,8
        STO $1,fp,16
        STO $2,fp,24
        GET $1,rL
        STO $1,fp,32
        SET $1,0
        PUT rL,$1
        SET $1,ax
        SET ax,0
@1      LDB $2,$1,0
        BZ $2,@2
        ADD ax,ax,1
        ADD $1,$1,1
        JMP @1
@2      LDO $1,fp,32
        PUT rL,$1
        LDO $2,fp,24
        LDO $1,fp,16
        LDO $0,fp,8
        SET sp,fp
        LDO fp,sp,0
        GO $0,$0,0
strlen  ENDF

puts    FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,4*8
        STO $0,fp,8
        STO $1,fp,16
        GET $1,rL
        STO $1,fp,24
        SET $1,0
        PUT rL,$1
        SET bx,1
        GO $0,putsf
        LDO $1,fp,24
        PUT rL,$1
        LDO $1,fp,16
        LDO $0,fp,8
        SET sp,fp
        LDO fp,sp,0
        GO $0,$0,0
puts    ENDF

putsf   FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,5*8
        STO $0,fp,8
        STO $1,fp,16
        STO $2,fp,24
        GET $1,rL
        STO $1,fp,32
        SET $1,0
        PUT rL,$1
        SET $2,bx
        SET $1,ax
        GO $0,strlen
        SET cx,ax
        SET bx,$1
        SET ax,$2
        GO $0,write
        LDO $1,fp,32
        PUT rL,$1
        LDO $2,fp,24
        LDO $1,fp,16
        LDO $0,fp,8
        SET sp,fp
        LDO fp,sp,0
        GO $0,$0,0
putsf   ENDF

putnl   FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,4*8
        STO $0,fp,8
        STO $1,fp,16
        GET $1,rL
        STO $1,fp,24
        SET $1,0
        PUT rL,$1
        SET ax,1
        GO $0,putnlf
        LDO $1,fp,24
        PUT rL,$1
        LDO $1,fp,16
        LDO $0,fp,8
        SET sp,fp
        LDO fp,sp,0
        GO $0,$0,0
putnl   ENDF

putnlf  FUNC
        STO fp,sp,0
        SET fp,sp
        INCL sp,4*8
        STO $0,fp,8
        STO $1,fp,16
        GET $1,rL
        STO $1,fp,24
        SET $1,0
        PUT rL,$1
        SET bx,ax
        LDOU ax,newline
        GO $0,putsf
        LDO $1,fp,24
        PUT rL,$1
        LDO $1,fp,16
        LDO $0,fp,8
        SET sp,fp
        LDO fp,sp,0
        GO $0,$0,0
putnlf  ENDF

.DATA

newline OCTA nl
nl      BYTE 10,0

