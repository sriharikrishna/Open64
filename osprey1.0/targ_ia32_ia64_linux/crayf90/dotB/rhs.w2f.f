C ***********************************************************
C Fortran file translated from WHIRL Thu Apr 25 13:47:05 2002
C ***********************************************************

      SUBROUTINE rhs()
      IMPLICIT NONE
C
C     **** Variables and functions ****
C
      REAL(8) FLUX(1 : 5, 1 : 33)
      INTEGER(4) I
      INTEGER(4) J
      INTEGER(4) K
      INTEGER(4) M
      REAL(8) Q
      REAL(8) TMP
      REAL(8) U21
      REAL(8) U21I
      REAL(8) U21IM1
      REAL(8) U21J
      REAL(8) U21JM1
      REAL(8) U21K
      REAL(8) U21KM1
      REAL(8) U31
      REAL(8) U31I
      REAL(8) U31IM1
      REAL(8) U31J
      REAL(8) U31JM1
      REAL(8) U31K
      REAL(8) U31KM1
      REAL(8) U41
      REAL(8) U41I
      REAL(8) U41IM1
      REAL(8) U41J
      REAL(8) U41JM1
      REAL(8) U41K
      REAL(8) U41KM1
      REAL(8) U51I
      REAL(8) U51IM1
      REAL(8) U51J
      REAL(8) U51JM1
      REAL(8) U51K
      REAL(8) U51KM1
      REAL(8) A0(1 : 5, 1 : 5, 1 : 33, 1 : 33, 1 : 33)
      REAL(8) B0(1 : 5, 1 : 5, 1 : 33, 1 : 33, 1 : 33)
      REAL(8) C0(1 : 5, 1 : 5, 1 : 33, 1 : 33, 1 : 33)
      REAL(8) D0(1 : 5, 1 : 5, 1 : 33, 1 : 33, 1 : 33)
      COMMON /cjac/ A0, B0, C0, D0
      REAL(8) CE0(1 : 5, 1 : 13)
      COMMON /cexact/ CE0
      INTEGER(4) NX0
      INTEGER(4) NY0
      INTEGER(4) NZ0
      INTEGER(4) II3
      INTEGER(4) II4
      INTEGER(4) JI3
      INTEGER(4) JI4
      INTEGER(4) KI3
      INTEGER(4) KI4
      INTEGER(4) ITWJ0
      REAL(8) DXI0
      REAL(8) DETA0
      REAL(8) DZETA0
      REAL(8) TX6
      REAL(8) TX4
      REAL(8) TX5
      REAL(8) TY6
      REAL(8) TY4
      REAL(8) TY5
      REAL(8) TZ6
      REAL(8) TZ4
      REAL(8) TZ5
      COMMON /cgcon/ NX0, NY0, NZ0, II3, II4, JI3, JI4, KI3, KI4, ITWJ0
     > , DXI0, DETA0, DZETA0, TX6, TX4, TX5, TY6, TY4, TY5, TZ6, TZ4,
     >  TZ5
      REAL(8) DX6
      REAL(8) DX7
      REAL(8) DX8
      REAL(8) DX9
      REAL(8) DX10
      REAL(8) DY6
      REAL(8) DY7
      REAL(8) DY8
      REAL(8) DY9
      REAL(8) DY10
      REAL(8) DZ6
      REAL(8) DZ7
      REAL(8) DZ8
      REAL(8) DZ9
      REAL(8) DZ10
      REAL(8) DSSP0
      COMMON /disp/ DX6, DX7, DX8, DX9, DX10, DY6, DY7, DY8, DY9, DY10,
     >  DZ6, DZ7, DZ8, DZ9, DZ10, DSSP0
      INTEGER(4) ITMAX0
      INTEGER(4) INVERT0
      REAL(8) DT0
      REAL(8) OMEGA0
      REAL(8) TOLRSD0(1 : 5)
      REAL(8) RSDNM0(1 : 5)
      REAL(8) ERRNM0(1 : 5)
      REAL(8) FRC0
      REAL(8) TTOTAL0
      COMMON /ctscon/ ITMAX0, INVERT0, DT0, OMEGA0, TOLRSD0, RSDNM0,
     >  ERRNM0, FRC0, TTOTAL0
      REAL(8) U0(1 : 5, 1 : 33, 1 : 33, 1 : 33)
      REAL(8) RSD0(1 : 5, 1 : 33, 1 : 33, 1 : 33)
      REAL(8) FRCT0(1 : 5, 1 : 33, 1 : 33, 1 : 33)
      COMMON /cvar/ U0, RSD0, FRCT0
      INTEGER(4) IPR0
      INTEGER(4) IOUT0
      INTEGER(4) INORM0
      COMMON /cprcon/ IPR0, IOUT0, INORM0
C
C     **** Temporary variables ****
C
      INTEGER(4) doloop_ub
      INTEGER(4) doloop_ub0
      INTEGER(4) doloop_ub1
      INTEGER(4) doloop_ub2
      INTEGER(4) doloop_ub3
      INTEGER(4) doloop_ub4
      INTEGER(4) doloop_ub5
      INTEGER(4) doloop_ub6
      INTEGER(4) doloop_ub7
      INTEGER(4) doloop_ub8
      INTEGER(4) doloop_ub9
      INTEGER(4) doloop_ub10
      INTEGER(4) doloop_ub11
      INTEGER(4) doloop_ub12
      INTEGER(4) doloop_ub13
      INTEGER(4) doloop_ub14
      INTEGER(4) doloop_ub15
      INTEGER(4) doloop_ub16
      INTEGER(4) doloop_ub17
      INTEGER(4) doloop_ub18
      INTEGER(4) doloop_ub19
      INTEGER(4) doloop_ub20
      INTEGER(4) doloop_ub21
      INTEGER(4) doloop_ub22
C
C     **** statements ****
C
      doloop_ub = NZ0
      DO K = 1, doloop_ub, 1
        doloop_ub0 = NY0
        DO J = 1, doloop_ub0, 1
          doloop_ub1 = NX0
          DO I = 1, doloop_ub1, 1
            DO M = 1, 5, 1
              RSD0(M + 1, I + 1, J + 1, K + 1) = (- FRCT0(M + 1, I + 1,
     >  J + 1, K + 1))
            END DO
          END DO
        END DO
      END DO
      doloop_ub2 = (NZ0 +(-1))
      DO K = 2, doloop_ub2, 1
        doloop_ub3 = (NY0 +(-1))
        DO J = 2, doloop_ub3, 1
          doloop_ub4 = NX0
          DO I = 1, doloop_ub4, 1
            FLUX(1, I + 1) = U0(2, I + 1, J + 1, K + 1)
            U21 = (U0(2, I + 1, J + 1, K + 1) / U0(1, I + 1, J + 1, K +
     >  1))
            Q = ((((((U0(2, I + 1, J + 1, K + 1) * U0(2, I + 1, J + 1,
     >  K + 1)) +(U0(3, I + 1, J + 1, K + 1) * U0(3, I + 1, J + 1, K +
     >  1))) +(U0(4, I + 1, J + 1, K + 1) * U0(4, I + 1, J + 1, K + 1))
     > )) * 5.0D-01) / U0(1, I + 1, J + 1, K + 1))
            FLUX(2, I + 1) = ((U0(2, I + 1, J + 1, K + 1) * U21) +(((U0
     > (5, I + 1, J + 1, K + 1) - Q)) * 4.00000000000000022204D-01))
            FLUX(3, I + 1) = (U0(3, I + 1, J + 1, K + 1) * U21)
            FLUX(4, I + 1) = (U0(4, I + 1, J + 1, K + 1) * U21)
            FLUX(5, I + 1) = (U21 *(((U0(5, I + 1, J + 1, K + 1) *
     >  1.39999999999999991118D00) -(Q * 4.00000000000000022204D-01))))
          END DO
          doloop_ub5 = (NX0 +(-1))
          DO I = 2, doloop_ub5, 1
            DO M = 1, 5, 1
              RSD0(M + 1, I + 1, J + 1, K + 1) = (RSD0(M + 1, I + 1, J
     >  + 1, K + 1) -(TX4 *((FLUX(M + 1, I + 1) - FLUX(M + 1, I + (-1))
     > ))))
            END DO
          END DO
          doloop_ub6 = NX0
          DO I = 2, doloop_ub6, 1
            TMP = (1D00 / U0(1, I + 1, J + 1, K + 1))
            U21I = (U0(2, I + 1, J + 1, K + 1) * TMP)
            U31I = (U0(3, I + 1, J + 1, K + 1) * TMP)
            U41I = (U0(4, I + 1, J + 1, K + 1) * TMP)
            U51I = (U0(5, I + 1, J + 1, K + 1) * TMP)
            TMP = (1D00 / U0(1, I + (-1), J + 1, K + 1))
            U21IM1 = (U0(2, I + (-1), J + 1, K + 1) * TMP)
            U31IM1 = (U0(3, I + (-1), J + 1, K + 1) * TMP)
            U41IM1 = (U0(4, I + (-1), J + 1, K + 1) * TMP)
            U51IM1 = (U0(5, I + (-1), J + 1, K + 1) * TMP)
            FLUX(2, I + 1) = ((TX5 * 1.33333333333333325932D00) *((U21I
     >  - U21IM1)))
            FLUX(3, I + 1) = (TX5 *((U31I - U31IM1)))
            FLUX(4, I + 1) = (TX5 *((U41I - U41IM1)))
            FLUX(5, I + 1) = ((((TX5 * -4.79999999999999871214D-01) *((
     > ((((U21I ** 2) +(U31I ** 2)) +(U41I ** 2))) -((((U21IM1 ** 2) +(
     > U31IM1 ** 2)) +(U41IM1 ** 2)))))) +((TX5 *
     >  1.66666666666666657415D-01) *(((U21I ** 2) -(U21IM1 ** 2))))) +
     > ((TX5 * 1.95999999999999974243D00) *((U51I - U51IM1))))
          END DO
          doloop_ub7 = (NX0 +(-1))
          DO I = 2, doloop_ub7, 1
            RSD0(1, I + 1, J + 1, K + 1) = (RSD0(1, I + 1, J + 1, K + 1
     > ) +((DX6 * TX6) *((U0(1, I + 1, J + 1, K + 1) +(U0(1, I + (-1),
     >  J + 1, K + 1) -(U0(1, I + 1, J + 1, K + 1) * 2.0D00))))))
            RSD0(2, I + 1, J + 1, K + 1) = ((RSD0(2, I + 1, J + 1, K +
     >  1) +((TX5 * 1.00000000000000005551D-01) *((FLUX(2, I + 1) -
     >  FLUX(2, I + 1))))) +((DX7 * TX6) *((U0(2, I + 1, J + 1, K + 1)
     >  +(U0(2, I + (-1), J + 1, K + 1) -(U0(2, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(3, I + 1, J + 1, K + 1) = ((RSD0(3, I + 1, J + 1, K +
     >  1) +((TX5 * 1.00000000000000005551D-01) *((FLUX(3, I + 1) -
     >  FLUX(3, I + 1))))) +((DX8 * TX6) *((U0(3, I + 1, J + 1, K + 1)
     >  +(U0(3, I + (-1), J + 1, K + 1) -(U0(3, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(4, I + 1, J + 1, K + 1) = ((RSD0(4, I + 1, J + 1, K +
     >  1) +((TX5 * 1.00000000000000005551D-01) *((FLUX(4, I + 1) -
     >  FLUX(4, I + 1))))) +((DX9 * TX6) *((U0(4, I + 1, J + 1, K + 1)
     >  +(U0(4, I + (-1), J + 1, K + 1) -(U0(4, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(5, I + 1, J + 1, K + 1) = ((RSD0(5, I + 1, J + 1, K +
     >  1) +((TX5 * 1.00000000000000005551D-01) *((FLUX(5, I + 1) -
     >  FLUX(5, I + 1))))) +((DX10 * TX6) *((U0(5, I + 1, J + 1, K + 1)
     >  +(U0(5, I + (-1), J + 1, K + 1) -(U0(5, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
          END DO
          DO M = 1, 5, 1
            RSD0(M + 1, 2, J + 1, K + 1) = (RSD0(M + 1, 2, J + 1, K + 1
     > ) -(DSSP0 *((U0(M + 1, 4, J + 1, K + 1) +((U0(M + 1, 2, J + 1, K
     >  + 1) * 5.0D00) -(U0(M + 1, 3, J + 1, K + 1) * 4.0D00))))))
            RSD0(M + 1, 3, J + 1, K + 1) = (RSD0(M + 1, 3, J + 1, K + 1
     > ) -(DSSP0 *((U0(M + 1, 5, J + 1, K + 1) +(((U0(M + 1, 2, J + 1,
     >  K + 1) * -4.0D00) +(U0(M + 1, 3, J + 1, K + 1) * 6.0D00)) -(U0(
     > M + 1, 4, J + 1, K + 1) * 4.0D00))))))
          END DO
          doloop_ub8 = (NX0 +(-3))
          DO I = 4, doloop_ub8, 1
            DO M = 1, 5, 1
              RSD0(M + 1, I + 1, J + 1, K + 1) = (RSD0(M + 1, I + 1, J
     >  + 1, K + 1) -(DSSP0 *((U0(M + 1, I + 2, J + 1, K + 1) +(((U0(M
     >  + 1, I + 1, J + 1, K + 1) * 6.0D00) +(U0(M + 1, I + (-2), J + 1
     > , K + 1) -(U0(M + 1, I + (-1), J + 1, K + 1) * 4.0D00))) -(U0(M
     >  + 1, I + 1, J + 1, K + 1) * 4.0D00))))))
            END DO
          END DO
          DO M = 1, 5, 1
            RSD0(M + 1, NX0 + (-2), J + 1, K + 1) = (RSD0(M + 1, NX0 +
     >  (-2), J + 1, K + 1) -(DSSP0 *((((U0(M + 1, NX0 + (-2), J + 1, K
     >  + 1) * 6.0D00) +(U0(M + 1, NX0 + (-4), J + 1, K + 1) -(U0(M + 1
     > , NX0 + (-3), J + 1, K + 1) * 4.0D00))) -(U0(M + 1, NX0 + (-1),
     >  J + 1, K + 1) * 4.0D00)))))
            RSD0(M + 1, NX0 + (-1), J + 1, K + 1) = (RSD0(M + 1, NX0 +
     >  (-1), J + 1, K + 1) -(DSSP0 *(((U0(M + 1, NX0 + (-1), J + 1, K
     >  + 1) * 5.0D00) +(U0(M + 1, NX0 + (-3), J + 1, K + 1) -(U0(M + 1
     > , NX0 + (-2), J + 1, K + 1) * 4.0D00))))))
          END DO
        END DO
      END DO
      doloop_ub9 = (NZ0 +(-1))
      DO K = 2, doloop_ub9, 1
        doloop_ub10 = (NX0 +(-1))
        DO I = 2, doloop_ub10, 1
          doloop_ub11 = NY0
          DO J = 1, doloop_ub11, 1
            FLUX(1, J + 1) = U0(3, I + 1, J + 1, K + 1)
            U31 = (U0(3, I + 1, J + 1, K + 1) / U0(1, I + 1, J + 1, K +
     >  1))
            Q = ((((((U0(2, I + 1, J + 1, K + 1) * U0(2, I + 1, J + 1,
     >  K + 1)) +(U0(3, I + 1, J + 1, K + 1) * U0(3, I + 1, J + 1, K +
     >  1))) +(U0(4, I + 1, J + 1, K + 1) * U0(4, I + 1, J + 1, K + 1))
     > )) * 5.0D-01) / U0(1, I + 1, J + 1, K + 1))
            FLUX(2, J + 1) = (U0(2, I + 1, J + 1, K + 1) * U31)
            FLUX(3, J + 1) = ((U0(3, I + 1, J + 1, K + 1) * U31) +(((U0
     > (5, I + 1, J + 1, K + 1) - Q)) * 4.00000000000000022204D-01))
            FLUX(4, J + 1) = (U0(4, I + 1, J + 1, K + 1) * U31)
            FLUX(5, J + 1) = (U31 *(((U0(5, I + 1, J + 1, K + 1) *
     >  1.39999999999999991118D00) -(Q * 4.00000000000000022204D-01))))
          END DO
          doloop_ub12 = (NY0 +(-1))
          DO J = 2, doloop_ub12, 1
            DO M = 1, 5, 1
              RSD0(M + 1, I + 1, J + 1, K + 1) = (RSD0(M + 1, I + 1, J
     >  + 1, K + 1) -(TY4 *((FLUX(M + 1, J + 1) - FLUX(M + 1, J + (-1))
     > ))))
            END DO
          END DO
          doloop_ub13 = NY0
          DO J = 2, doloop_ub13, 1
            TMP = (1D00 / U0(1, I + 1, J + 1, K + 1))
            U21J = (U0(2, I + 1, J + 1, K + 1) * TMP)
            U31J = (U0(3, I + 1, J + 1, K + 1) * TMP)
            U41J = (U0(4, I + 1, J + 1, K + 1) * TMP)
            U51J = (U0(5, I + 1, J + 1, K + 1) * TMP)
            TMP = (1D00 / U0(1, I + 1, J + (-1), K + 1))
            U21JM1 = (U0(2, I + 1, J + (-1), K + 1) * TMP)
            U31JM1 = (U0(3, I + 1, J + (-1), K + 1) * TMP)
            U41JM1 = (U0(4, I + 1, J + (-1), K + 1) * TMP)
            U51JM1 = (U0(5, I + 1, J + (-1), K + 1) * TMP)
            FLUX(2, J + 1) = (TY5 *((U21J - U21JM1)))
            FLUX(3, J + 1) = ((TY5 * 1.33333333333333325932D00) *((U31J
     >  - U31JM1)))
            FLUX(4, J + 1) = (TY5 *((U41J - U41JM1)))
            FLUX(5, J + 1) = ((((TY5 * -4.79999999999999871214D-01) *((
     > ((((U21J ** 2) +(U31J ** 2)) +(U41J ** 2))) -((((U21JM1 ** 2) +(
     > U31JM1 ** 2)) +(U41JM1 ** 2)))))) +((TY5 *
     >  1.66666666666666657415D-01) *(((U31J ** 2) -(U31JM1 ** 2))))) +
     > ((TY5 * 1.95999999999999974243D00) *((U51J - U51JM1))))
          END DO
          doloop_ub14 = (NY0 +(-1))
          DO J = 2, doloop_ub14, 1
            RSD0(1, I + 1, J + 1, K + 1) = (RSD0(1, I + 1, J + 1, K + 1
     > ) +((DY6 * TY6) *((U0(1, I + 1, J + 1, K + 1) +(U0(1, I + 1, J +
     >  (-1), K + 1) -(U0(1, I + 1, J + 1, K + 1) * 2.0D00))))))
            RSD0(2, I + 1, J + 1, K + 1) = ((RSD0(2, I + 1, J + 1, K +
     >  1) +((TY5 * 1.00000000000000005551D-01) *((FLUX(2, J + 1) -
     >  FLUX(2, J + 1))))) +((DY7 * TY6) *((U0(2, I + 1, J + 1, K + 1)
     >  +(U0(2, I + 1, J + (-1), K + 1) -(U0(2, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(3, I + 1, J + 1, K + 1) = ((RSD0(3, I + 1, J + 1, K +
     >  1) +((TY5 * 1.00000000000000005551D-01) *((FLUX(3, J + 1) -
     >  FLUX(3, J + 1))))) +((DY8 * TY6) *((U0(3, I + 1, J + 1, K + 1)
     >  +(U0(3, I + 1, J + (-1), K + 1) -(U0(3, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(4, I + 1, J + 1, K + 1) = ((RSD0(4, I + 1, J + 1, K +
     >  1) +((TY5 * 1.00000000000000005551D-01) *((FLUX(4, J + 1) -
     >  FLUX(4, J + 1))))) +((DY9 * TY6) *((U0(4, I + 1, J + 1, K + 1)
     >  +(U0(4, I + 1, J + (-1), K + 1) -(U0(4, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(5, I + 1, J + 1, K + 1) = ((RSD0(5, I + 1, J + 1, K +
     >  1) +((TY5 * 1.00000000000000005551D-01) *((FLUX(5, J + 1) -
     >  FLUX(5, J + 1))))) +((DY10 * TY6) *((U0(5, I + 1, J + 1, K + 1)
     >  +(U0(5, I + 1, J + (-1), K + 1) -(U0(5, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
          END DO
          DO M = 1, 5, 1
            RSD0(M + 1, I + 1, 2, K + 1) = (RSD0(M + 1, I + 1, 2, K + 1
     > ) -(DSSP0 *((U0(M + 1, I + 1, 4, K + 1) +((U0(M + 1, I + 1, 2, K
     >  + 1) * 5.0D00) -(U0(M + 1, I + 1, 3, K + 1) * 4.0D00))))))
            RSD0(M + 1, I + 1, 3, K + 1) = (RSD0(M + 1, I + 1, 3, K + 1
     > ) -(DSSP0 *((U0(M + 1, I + 1, 5, K + 1) +(((U0(M + 1, I + 1, 2,
     >  K + 1) * -4.0D00) +(U0(M + 1, I + 1, 3, K + 1) * 6.0D00)) -(U0(
     > M + 1, I + 1, 4, K + 1) * 4.0D00))))))
          END DO
          doloop_ub15 = (NY0 +(-3))
          DO J = 4, doloop_ub15, 1
            DO M = 1, 5, 1
              RSD0(M + 1, I + 1, J + 1, K + 1) = (RSD0(M + 1, I + 1, J
     >  + 1, K + 1) -(DSSP0 *((U0(M + 1, I + 1, J + 2, K + 1) +(((U0(M
     >  + 1, I + 1, J + 1, K + 1) * 6.0D00) +(U0(M + 1, I + 1, J + (-2)
     > , K + 1) -(U0(M + 1, I + 1, J + (-1), K + 1) * 4.0D00))) -(U0(M
     >  + 1, I + 1, J + 1, K + 1) * 4.0D00))))))
            END DO
          END DO
          DO M = 1, 5, 1
            RSD0(M + 1, I + 1, NY0 + (-2), K + 1) = (RSD0(M + 1, I + 1,
     >  NY0 + (-2), K + 1) -(DSSP0 *((((U0(M + 1, I + 1, NY0 + (-2), K
     >  + 1) * 6.0D00) +(U0(M + 1, I + 1, NY0 + (-4), K + 1) -(U0(M + 1
     > , I + 1, NY0 + (-3), K + 1) * 4.0D00))) -(U0(M + 1, I + 1, NY0 +
     >  (-1), K + 1) * 4.0D00)))))
            RSD0(M + 1, I + 1, NY0 + (-1), K + 1) = (RSD0(M + 1, I + 1,
     >  NY0 + (-1), K + 1) -(DSSP0 *(((U0(M + 1, I + 1, NY0 + (-1), K +
     >  1) * 5.0D00) +(U0(M + 1, I + 1, NY0 + (-3), K + 1) -(U0(M + 1,
     >  I + 1, NY0 + (-2), K + 1) * 4.0D00))))))
          END DO
        END DO
      END DO
      doloop_ub16 = (NY0 +(-1))
      DO J = 2, doloop_ub16, 1
        doloop_ub17 = (NX0 +(-1))
        DO I = 2, doloop_ub17, 1
          doloop_ub18 = NZ0
          DO K = 1, doloop_ub18, 1
            FLUX(1, K + 1) = U0(4, I + 1, J + 1, K + 1)
            U41 = (U0(4, I + 1, J + 1, K + 1) / U0(1, I + 1, J + 1, K +
     >  1))
            Q = ((((((U0(2, I + 1, J + 1, K + 1) * U0(2, I + 1, J + 1,
     >  K + 1)) +(U0(3, I + 1, J + 1, K + 1) * U0(3, I + 1, J + 1, K +
     >  1))) +(U0(4, I + 1, J + 1, K + 1) * U0(4, I + 1, J + 1, K + 1))
     > )) * 5.0D-01) / U0(1, I + 1, J + 1, K + 1))
            FLUX(2, K + 1) = (U0(2, I + 1, J + 1, K + 1) * U41)
            FLUX(3, K + 1) = (U0(3, I + 1, J + 1, K + 1) * U41)
            FLUX(4, K + 1) = ((U0(4, I + 1, J + 1, K + 1) * U41) +(((U0
     > (5, I + 1, J + 1, K + 1) - Q)) * 4.00000000000000022204D-01))
            FLUX(5, K + 1) = (U41 *(((U0(5, I + 1, J + 1, K + 1) *
     >  1.39999999999999991118D00) -(Q * 4.00000000000000022204D-01))))
          END DO
          doloop_ub19 = (NZ0 +(-1))
          DO K = 2, doloop_ub19, 1
            DO M = 1, 5, 1
              RSD0(M + 1, I + 1, J + 1, K + 1) = (RSD0(M + 1, I + 1, J
     >  + 1, K + 1) -(TZ4 *((FLUX(M + 1, K + 1) - FLUX(M + 1, K + (-1))
     > ))))
            END DO
          END DO
          doloop_ub20 = NZ0
          DO K = 2, doloop_ub20, 1
            TMP = (1D00 / U0(1, I + 1, J + 1, K + 1))
            U21K = (U0(2, I + 1, J + 1, K + 1) * TMP)
            U31K = (U0(3, I + 1, J + 1, K + 1) * TMP)
            U41K = (U0(4, I + 1, J + 1, K + 1) * TMP)
            U51K = (U0(5, I + 1, J + 1, K + 1) * TMP)
            TMP = (1D00 / U0(1, I + 1, J + 1, K + (-1)))
            U21KM1 = (U0(2, I + 1, J + 1, K + (-1)) * TMP)
            U31KM1 = (U0(3, I + 1, J + 1, K + (-1)) * TMP)
            U41KM1 = (U0(4, I + 1, J + 1, K + (-1)) * TMP)
            U51KM1 = (U0(5, I + 1, J + 1, K + (-1)) * TMP)
            FLUX(2, K + 1) = (TZ5 *((U21K - U21KM1)))
            FLUX(3, K + 1) = (TZ5 *((U31K - U31KM1)))
            FLUX(4, K + 1) = ((TZ5 * 1.33333333333333325932D00) *((U41K
     >  - U41KM1)))
            FLUX(5, K + 1) = ((((TZ5 * -4.79999999999999871214D-01) *((
     > ((((U21K ** 2) +(U31K ** 2)) +(U41K ** 2))) -((((U21KM1 ** 2) +(
     > U31KM1 ** 2)) +(U41KM1 ** 2)))))) +((TZ5 *
     >  1.66666666666666657415D-01) *(((U41K ** 2) -(U41KM1 ** 2))))) +
     > ((TZ5 * 1.95999999999999974243D00) *((U51K - U51KM1))))
          END DO
          doloop_ub21 = (NZ0 +(-1))
          DO K = 2, doloop_ub21, 1
            RSD0(1, I + 1, J + 1, K + 1) = (RSD0(1, I + 1, J + 1, K + 1
     > ) +((DZ6 * TZ6) *((U0(1, I + 1, J + 1, K + 1) +(U0(1, I + 1, J +
     >  1, K + (-1)) -(U0(1, I + 1, J + 1, K + 1) * 2.0D00))))))
            RSD0(2, I + 1, J + 1, K + 1) = ((RSD0(2, I + 1, J + 1, K +
     >  1) +((TZ5 * 1.00000000000000005551D-01) *((FLUX(2, K + 1) -
     >  FLUX(2, K + 1))))) +((DZ7 * TZ6) *((U0(2, I + 1, J + 1, K + 1)
     >  +(U0(2, I + 1, J + 1, K + (-1)) -(U0(2, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(3, I + 1, J + 1, K + 1) = ((RSD0(3, I + 1, J + 1, K +
     >  1) +((TZ5 * 1.00000000000000005551D-01) *((FLUX(3, K + 1) -
     >  FLUX(3, K + 1))))) +((DZ8 * TZ6) *((U0(3, I + 1, J + 1, K + 1)
     >  +(U0(3, I + 1, J + 1, K + (-1)) -(U0(3, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(4, I + 1, J + 1, K + 1) = ((RSD0(4, I + 1, J + 1, K +
     >  1) +((TZ5 * 1.00000000000000005551D-01) *((FLUX(4, K + 1) -
     >  FLUX(4, K + 1))))) +((DZ9 * TZ6) *((U0(4, I + 1, J + 1, K + 1)
     >  +(U0(4, I + 1, J + 1, K + (-1)) -(U0(4, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
            RSD0(5, I + 1, J + 1, K + 1) = ((RSD0(5, I + 1, J + 1, K +
     >  1) +((TZ5 * 1.00000000000000005551D-01) *((FLUX(5, K + 1) -
     >  FLUX(5, K + 1))))) +((DZ10 * TZ6) *((U0(5, I + 1, J + 1, K + 1)
     >  +(U0(5, I + 1, J + 1, K + (-1)) -(U0(5, I + 1, J + 1, K + 1) *
     >  2.0D00))))))
          END DO
          DO M = 1, 5, 1
            RSD0(M + 1, I + 1, J + 1, 2) = (RSD0(M + 1, I + 1, J + 1, 2
     > ) -(DSSP0 *((U0(M + 1, I + 1, J + 1, 4) +((U0(M + 1, I + 1, J +
     >  1, 2) * 5.0D00) -(U0(M + 1, I + 1, J + 1, 3) * 4.0D00))))))
            RSD0(M + 1, I + 1, J + 1, 3) = (RSD0(M + 1, I + 1, J + 1, 3
     > ) -(DSSP0 *((U0(M + 1, I + 1, J + 1, 5) +(((U0(M + 1, I + 1, J +
     >  1, 2) * -4.0D00) +(U0(M + 1, I + 1, J + 1, 3) * 6.0D00)) -(U0(M
     >  + 1, I + 1, J + 1, 4) * 4.0D00))))))
          END DO
          doloop_ub22 = (NZ0 +(-3))
          DO K = 4, doloop_ub22, 1
            DO M = 1, 5, 1
              RSD0(M + 1, I + 1, J + 1, K + 1) = (RSD0(M + 1, I + 1, J
     >  + 1, K + 1) -(DSSP0 *((U0(M + 1, I + 1, J + 1, K + 2) +(((U0(M
     >  + 1, I + 1, J + 1, K + 1) * 6.0D00) +(U0(M + 1, I + 1, J + 1, K
     >  + (-2)) -(U0(M + 1, I + 1, J + 1, K + (-1)) * 4.0D00))) -(U0(M
     >  + 1, I + 1, J + 1, K + 1) * 4.0D00))))))
            END DO
          END DO
          DO M = 1, 5, 1
            RSD0(M + 1, I + 1, J + 1, NZ0 + (-2)) = (RSD0(M + 1, I + 1,
     >  J + 1, NZ0 + (-2)) -(DSSP0 *((((U0(M + 1, I + 1, J + 1, NZ0 + (
     > -2)) * 6.0D00) +(U0(M + 1, I + 1, J + 1, NZ0 + (-4)) -(U0(M + 1,
     >  I + 1, J + 1, NZ0 + (-3)) * 4.0D00))) -(U0(M + 1, I + 1, J + 1,
     >  NZ0 + (-1)) * 4.0D00)))))
            RSD0(M + 1, I + 1, J + 1, NZ0 + (-1)) = (RSD0(M + 1, I + 1,
     >  J + 1, NZ0 + (-1)) -(DSSP0 *(((U0(M + 1, I + 1, J + 1, NZ0 + (
     > -1)) * 5.0D00) +(U0(M + 1, I + 1, J + 1, NZ0 + (-3)) -(U0(M + 1,
     >  I + 1, J + 1, NZ0 + (-2)) * 4.0D00))))))
          END DO
        END DO
      END DO
      RETURN
      END SUBROUTINE
