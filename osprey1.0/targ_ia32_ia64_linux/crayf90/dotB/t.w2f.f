C ***********************************************************
C Fortran file translated from WHIRL Thu Apr 25 13:46:48 2002
C ***********************************************************

      PROGRAM MAIN
      IMPLICIT NONE
C
C     **** Variables and functions ****
C
      REAL(8) A(1 : 100)
      REAL(8) B(1 : 100)
      INTEGER(4) I
C
C     **** statements ****
C
      DO I = 1, 100, 1
        A(I + 1) = (B(I + 1) * 2.0D00)
      END DO
      WRITE( ,  , IOSTAT = , REC = , SIZE = ) A(I + 1)
      
      END
