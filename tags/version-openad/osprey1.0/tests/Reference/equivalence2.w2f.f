C ***********************************************************
C Fortran file translated from WHIRL Wed Oct 28 17:21:18 2009
C ***********************************************************
C ***********************************************************

      PROGRAM p
      use w2f__types
      IMPLICIT NONE
C
C     **** Global Variables & Derived Type Definitions ****
C
      SAVE /cb/
      COMMON /cb/ V1, V2
      REAL(w2f__4) F1
      REAL(w2f__4) V1
      REAL(w2f__4) F2
      REAL(w2f__4) V2
      EQUIVALENCE(tmp0(1), F1)
      EQUIVALENCE(tmp0(1), V1)
      EQUIVALENCE(tmp0(5), F2)
C
C     **** Temporary Variables ****
C
      INTEGER(w2f__i1) tmp0(1 : 8)
C
C     **** Statements ****
C
      V1 = 1.0
      V2 = 2.0
      IF(F2 .GT. 1.5) THEN
        WRITE(*, *) 'OK'
      ELSE
        WRITE(*, *) F2
      ENDIF
      
      END PROGRAM
