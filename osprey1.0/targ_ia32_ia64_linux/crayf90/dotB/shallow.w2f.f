C ***********************************************************
C Fortran file translated from WHIRL Thu Apr 25 13:47:27 2002
C ***********************************************************

      MODULE timer
      IMPLICIT NONE
C
C     **** Variables and functions ****
C
      
      INTEGER(4) STARTING_COUNT
      LOGICAL(4) SYSTEM_CLOCK_NOT_YET_CALLED
      
      REAL(4) A
      REAL(4) ALPHA
      REAL(4) DI
      REAL(4) DJ
      REAL(4) DT
      REAL(4) DX
      REAL(4) DY
      REAL(4) EL
      REAL(4) FSDX
      REAL(4) FSDY
      INTEGER(4) ITMAX
      INTEGER(4) M
      INTEGER(4) MP1
      INTEGER(4) MPRINT
      INTEGER(4) N
      INTEGER(4) NP1
      REAL(4) PCF
      REAL(4) PI
      REAL(4) TBEG
      REAL(4) TDT
      REAL(4) TDTS8
      REAL(4) TDTSDX
      REAL(4) TDTSDY
      REAL(4) TPI
C
C     **** statements ****
C
      CONTAINS

        SUBROUTINE PRINT_TIME()
        IMPLICIT NONE
C
C       **** Variables and functions ****
C
        CHARACTER*8 DATE
        CHARACTER*10 TIME
        CHARACTER*22 t$1
        CHARACTER*22 ch_str_temp_0
        EXTERNAL _F90_DATE_AND_TIME
        
        INTEGER(4) STARTING_COUNT
        LOGICAL(4) SYSTEM_CLOCK_NOT_YET_CALLED
        
        REAL(4) A
        REAL(4) ALPHA
        REAL(4) DI
        REAL(4) DJ
        REAL(4) DT
        REAL(4) DX
        REAL(4) DY
        REAL(4) EL
        REAL(4) FSDX
        REAL(4) FSDY
        INTEGER(4) ITMAX
        INTEGER(4) M
        INTEGER(4) MP1
        INTEGER(4) MPRINT
        INTEGER(4) N
        INTEGER(4) NP1
        REAL(4) PCF
        REAL(4) PI
        REAL(4) TBEG
        REAL(4) TDT
        REAL(4) TDTS8
        REAL(4) TDTSDX
        REAL(4) TDTSDY
        REAL(4) TPI
C
C       **** statements ****
C
        CALL _F90_DATE_AND_TIME(DATE, TIME,  ,  )
        ch_str_temp_0 = DATE(4_8 : 4_8 + 2) // '/' // DATE(6_8 : 6_8 +
     >  2) // '/' // DATE(2_8 : 2_8 + 2) // '  ' // TIME(0_8 : 2) //
     >  ':' // TIME(2_8 : 2_8 + 2) // ':' // TIME(4_8 : 4_8 + 6)
        t$1 = ch_str_temp_0
        WRITE( ,  , IOSTAT = , REC = , SIZE = ) t$1
        RETURN
        END SUBROUTINE

        REAL(4) FUNCTION CPUTIME()
        IMPLICIT NONE
C
C       **** Variables and functions ****
C
        REAL(4) TIME
        
        EXTERNAL secnds
        REAL(4) secnds
        INTEGER(4) STARTING_COUNT
        LOGICAL(4) SYSTEM_CLOCK_NOT_YET_CALLED
        
        REAL(4) A
        REAL(4) ALPHA
        REAL(4) DI
        REAL(4) DJ
        REAL(4) DT
        REAL(4) DX
        REAL(4) DY
        REAL(4) EL
        REAL(4) FSDX
        REAL(4) FSDY
        INTEGER(4) ITMAX
        INTEGER(4) M
        INTEGER(4) MP1
        INTEGER(4) MPRINT
        INTEGER(4) N
        INTEGER(4) NP1
        REAL(4) PCF
        REAL(4) PI
        REAL(4) TBEG
        REAL(4) TDT
        REAL(4) TDTS8
        REAL(4) TDTSDX
        REAL(4) TDTSDY
        REAL(4) TPI
C
C       **** statements ****
C
        TIME = secnds(0.0)
        CPUTIME = TIME
        RETURN
        END FUNCTION
      END

      MODULE cons
      IMPLICIT NONE
C
C     **** Variables and functions ****
C
      
      INTEGER(4) STARTING_COUNT
      LOGICAL(4) SYSTEM_CLOCK_NOT_YET_CALLED
      
      REAL(4) A
      REAL(4) ALPHA
      REAL(4) DI
      REAL(4) DJ
      REAL(4) DT
      REAL(4) DX
      REAL(4) DY
      REAL(4) EL
      REAL(4) FSDX
      REAL(4) FSDY
      INTEGER(4) ITMAX
      INTEGER(4) M
      INTEGER(4) MP1
      INTEGER(4) MPRINT
      INTEGER(4) N
      INTEGER(4) NP1
      REAL(4) PCF
      REAL(4) PI
      REAL(4) TBEG
      REAL(4) TDT
      REAL(4) TDTS8
      REAL(4) TDTSDX
      REAL(4) TDTSDY
      REAL(4) TPI
C
C     **** statements ****
C
      END MODULE
