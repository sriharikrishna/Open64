C ***********************************************************
C Fortran file translated from WHIRL Tue Aug 25 15:42:37 2009
C ***********************************************************
C ***********************************************************

      PROGRAM transp
      use w2f__types
      IMPLICIT NONE
C
C     **** Local Variables and Functions ****
C
      
      EQUIVALENCE(tmp0(1), t__1)
      EQUIVALENCE(tmp0(1), A)
      INTEGER(w2f__i4) t__1(1 : 4)
      SAVE t__1
      INTEGER(w2f__i4) A(1 : 2, 1 : 2)
      SAVE A
      INTEGER(w2f__i4) B(1 : 2, 1 : 2)
C
C     **** Temporary Variables ****
C
      INTEGER(w2f__i1) tmp0(1 : 16)
      INTEGER(w2f__i8) tmp1
C
C     **** Initializers ****
C
      DATA(t__1(tmp1), tmp1 = 1, 4, 1) / 1, 2, 3, 4 /
C
C     **** Statements ****
C
      B(1 : 2, 1 : 2) = TRANSPOSE(A)
      IF(B(1, 2) .eq. 2) THEN
        WRITE(*, *) 'OK'
      ELSE
        WRITE(*, *) B, A
      ENDIF
      
      END PROGRAM
