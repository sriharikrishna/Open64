C ***********************************************************
C Fortran file translated from WHIRL Thu Apr 25 13:46:42 2002
C ***********************************************************

      PROGRAM MAIN
      IMPLICIT NONE
C
C     **** Variables and functions ****
C
      REAL(8) A(1 : 100)
      REAL(8) B(1 : 100)
      INTEGER(4) I
      
      INTEGER(4) jak
C
C     **** statements ****
C
      DO I = 1, 100, 1
        A(I + 1) = (DBLE(jak(I)) * B(I + 1))
      END DO
      WRITE( ,  , IOSTAT = , REC = , SIZE = ) A(I + 1)
      
      END

      INTEGER(4) FUNCTION jak(A)
      IMPLICIT NONE
      INTEGER(4) A
C
C     **** statements ****
C
      IF(A .LT. 15) THEN
        jak = A
      ELSE
        jak = 2
      ENDIF
      RETURN
      END FUNCTION
