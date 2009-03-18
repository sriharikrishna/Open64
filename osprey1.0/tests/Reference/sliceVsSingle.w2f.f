C ***********************************************************
C Fortran file translated from WHIRL Wed Mar 18 16:53:15 2009
C ***********************************************************
C ***********************************************************

      SUBROUTINE foo(X)
      use w2f__types
      IMPLICIT NONE
C
C     **** Parameters and Result ****
C
      INTEGER(w2f__i4) X
C
C     **** Statements ****
C
      X = X + 1
      RETURN
      END SUBROUTINE

      PROGRAM arrays3
      use w2f__types
      IMPLICIT NONE
C
C     **** Local Variables and Functions ****
C
      EXTERNAL foo
      INTEGER(w2f__i4) X(1 : 2)
C
C     **** Statements ****
C
      X(1) = 1
      CALL foo(X(1))
      IF(X(1) .eq. 2) THEN
        WRITE(*, *) 'OK'
      ELSE
        WRITE(*, *) 'failed'
      ENDIF
      
      END PROGRAM
