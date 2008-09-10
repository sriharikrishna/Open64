C ***********************************************************
C Fortran file translated from WHIRL Wed Sep 10 11:38:57 2008
C ***********************************************************
C ***********************************************************

      MODULE bla
      use w2f__types
      IMPLICIT NONE
      SAVE
C
C     **** Statements ****
C
      END MODULE

      SUBROUTINE head(X, Y)
      use w2f__types
      IMPLICIT NONE
C
C     **** Parameters and Result ****
C
      REAL(w2f__8) X(1 :, 1 :)
      REAL(w2f__8) Y(1 : 10)
C
C     **** Statements ****
C
      Y(1 : 10) = RESHAPE(X, SHAPE(Y))
      WRITE(*, *) 'OK'
      RETURN
      END SUBROUTINE

      PROGRAM main
      use w2f__types
      IMPLICIT NONE
C
C     **** Local Variables and Functions ****
C
      EXTERNAL head
      REAL(w2f__8) X(1 : 2, 1 : 5)
      REAL(w2f__8) Y(1 : 10)
C
C     **** Statements ****
C
      CALL head(X, Y)
      
      END PROGRAM
