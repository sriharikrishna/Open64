C ***********************************************************
C Fortran file translated from WHIRL Thu Apr 25 13:46:52 2002
C ***********************************************************

      PROGRAM MAIN
      IMPLICIT NONE
C
C     **** Variables and functions ****
C
      INTEGER(4) I
      INTEGER(4) select_expr_temp_0
C
C     **** statements ****
C
      I = 7
      select_expr_temp_0 = I
      IF(.NOT.(select_expr_temp_0 .GE. 1)) GO TO 5
      IF(.NOT.(select_expr_temp_0 .LE. 9)) GO TO 5
      select_expr_temp_0 = 1
5     CONTINUE
      IF(.NOT.(select_expr_temp_0 .GE. 10)) GO TO 6
      IF(.NOT.(select_expr_temp_0 .LE. 20)) GO TO 6
      select_expr_temp_0 = 10
6     CONTINUE
      IF(select_expr_temp_0 .EQ. 0) GO TO 2
      IF(select_expr_temp_0 .EQ. 1) GO TO 3
      IF(select_expr_temp_0 .EQ. 10) GO TO 4
      GO TO 1
2     CONTINUE
      WRITE( ,  , IOSTAT = , REC = , SIZE = ) 'case <0>'
      GO TO 1
3     CONTINUE
      WRITE( ,  , IOSTAT = , REC = , SIZE = ) 'case <1:9>'
      GO TO 1
4     CONTINUE
      WRITE( ,  , IOSTAT = , REC = , SIZE = ) 'case <10:20>'
1     CONTINUE
      
      END
