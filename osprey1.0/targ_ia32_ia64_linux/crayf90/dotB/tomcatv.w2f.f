C ***********************************************************
C Fortran file translated from WHIRL Thu Apr 25 13:47:09 2002
C ***********************************************************

      PROGRAM MAIN
      IMPLICIT NONE
C
C     **** Variables and functions ****
C
      REAL(8) A
      REAL(8) AA(1 : 257, 1 : 257)
      REAL(8) ABX
      REAL(8) ABY
      REAL(8) ALFA
      REAL(8) B
      REAL(8) C
      REAL(8) D(1 : 257, 1 : 257)
      REAL(8) DD(1 : 257, 1 : 257)
      REAL(8) DMAX
      REAL(8) DXCM
      REAL(8) DYCM
      REAL(8) EPS
      REAL(8) H
      INTEGER(4) I
      INTEGER(4) I1P
      INTEGER(4) I2M
      INTEGER(4) IM
      INTEGER(4) IP
      INTEGER(4) IRXM
      INTEGER(4) IRYM
      INTEGER(4) IXCM
      INTEGER(4) IYCM
      INTEGER(4) J
      INTEGER(4) J1P
      INTEGER(4) J2M
      INTEGER(4) JM
      INTEGER(4) JP
      INTEGER(4) JRXM
      INTEGER(4) JRYM
      INTEGER(4) JXCM
      INTEGER(4) JYCM
      INTEGER(4) K
      INTEGER(4) L
      INTEGER(4) LL
      INTEGER(4) LMAX
      INTEGER(4) M
      REAL(8) PXX
      REAL(8) PXY
      REAL(8) PYY
      REAL(8) QI
      REAL(8) QJ
      REAL(8) QXX
      REAL(8) QXY
      REAL(8) QYY
      REAL(8) R
      REAL(8) REL
      REAL(8) RELFA
      REAL(8) RX(1 : 257, 1 : 257)
      REAL(8) RXM
      REAL(8) RY(1 : 257, 1 : 257)
      REAL(8) RYM
      REAL(8) X(1 : 257, 1 : 257)
      REAL(8) XX
      REAL(8) XY
      REAL(8) Y(1 : 257, 1 : 257)
      REAL(8) YX
      REAL(8) YY
      REAL(8) S1ALT0
      COMMON /zeit/ S1ALT0
      EXTERNAL FLOAT
      REAL(4) FLOAT
      EXTERNAL ABS
      REAL(8) ABS
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
C
C     **** statements ****
C
      S1ALT0 = 0.0D00
      ALFA = 1.00000001490116119385D-01
      RELFA = 9.80000019073486328125D-01
      LMAX = 100
      EPS = 5.0000000584304871154D-08
      H = DBLE((1E00 / FLOAT(256)))
      REL = (2.0D00 / RELFA)
      DO I = 1, 257, 1
30      CONTINUE
        X(I + 1, 1) = DBLE((FLOAT((I +(-1))) / FLOAT(256)))
      END DO
      DO I = 1, 257, 1
        X(I + 1, 257) = X(I + 1, 1)
        X(1, I + 1) = 0.0D00
        X(257, I + 1) = 1.0D00
        Y(I + 1, 1) = 0.0D00
        Y(1, I + 1) = X(I + 1, 1)
        Y(257, I + 1) = (X(I + 1, 1) * ALFA)
1       CONTINUE
      END DO
      DO I = 1, 257, 1
12      CONTINUE
        Y(I + 1, 257) = ((X(I + 1, 1) * Y(257, 257)) +(Y(1, 257) *((
     > 1.0D00 - X(I + 1, 1)))))
      END DO
      DO J = 2, 256, 1
        DO I = 2, 256, 1
          X(I + 1, J + 1) = (X(I + 1, 1) * 8.99999976158142089844D-01)
          Y(I + 1, J + 1) = ((((X(I + 1, 1) * Y(257, J + 1)) +(Y(1, J +
     >  1) *((1.0D00 - X(I + 1, 1)))))) * 8.99999976158142089844D-01)
24        CONTINUE
        END DO
      END DO
      I1P = 2
      J1P = 2
      I2M = 256
      J2M = 256
      LL = 0
10    CONTINUE
      IXCM = 0
      JXCM = 0
      DXCM = 0.0D00
      IYCM = 0
      JYCM = 0
      DYCM = 0.0D00
      IRXM = 0
      JRXM = 0
      RXM = 0.0D00
      IRYM = 0
      JRYM = 0
      RYM = 0.0D00
      M = 0
      doloop_ub = J2M
      DO J = J1P, doloop_ub, 1
        JP = (J + 1)
        JM = (J +(-1))
        M = (M + 1)
        doloop_ub0 = I2M
        DO I = I1P, doloop_ub0, 1
          IP = (I + 1)
          IM = (I +(-1))
          XX = (X(IP + 1, J + 1) - X(IM + 1, J + 1))
          YX = (Y(IP + 1, J + 1) - Y(IM + 1, J + 1))
          XY = (X(I + 1, JP + 1) - X(I + 1, JM + 1))
          YY = (Y(I + 1, JP + 1) - Y(I + 1, JM + 1))
          A = ((((XY * XY) +(YY * YY))) * 2.5D-01)
          B = ((((XX * XX) +(YX * YX))) * 2.5D-01)
          C = ((((XX * XY) +(YX * YY))) * 1.25D-01)
          QI = 0.0D00
          QJ = 0.0D00
          AA(I + 1, M + 1) = (- B)
          DD(I + 1, M + 1) = ((B + B) +(A * REL))
          PXX = (X(IM + 1, J + 1) +(X(IP + 1, J + 1) -(X(I + 1, J + 1)
     >  * 2.0D00)))
          QXX = (Y(IM + 1, J + 1) +(Y(IP + 1, J + 1) -(Y(I + 1, J + 1)
     >  * 2.0D00)))
          PYY = (X(I + 1, JM + 1) +(X(I + 1, JP + 1) -(X(I + 1, J + 1)
     >  * 2.0D00)))
          QYY = (Y(I + 1, JM + 1) +(Y(I + 1, JP + 1) -(Y(I + 1, J + 1)
     >  * 2.0D00)))
          PXY = (X(IM + 1, JM + 1) +((X(IP + 1, JP + 1) - X(IP + 1, JM
     >  + 1)) - X(IM + 1, JP + 1)))
          QXY = (Y(IM + 1, JM + 1) +((Y(IP + 1, JP + 1) - Y(IP + 1, JM
     >  + 1)) - Y(IM + 1, JP + 1)))
          RX(I + 1, M + 1) = (((QI * XX) +(((A * PXX) +(B * PYY)) -(C *
     >  PXY))) +(QJ * XY))
          RY(I + 1, M + 1) = (((QI * YX) +(((A * QXX) +(B * QYY)) -(C *
     >  QXY))) +(QJ * YY))
14        CONTINUE
        END DO
26      CONTINUE
      END DO
      doloop_ub1 = M
      DO J = 1, doloop_ub1, 1
        doloop_ub2 = I2M
        DO I = I1P, doloop_ub2, 1
          IF(ABS(RX(I + 1, J + 1)) .LT. ABS(RXM)) GO TO 16
          RXM = RX(I + 1, J + 1)
          IRXM = I
          JRXM = J
16        CONTINUE
          IF(ABS(RY(I + 1, J + 1)) .LT. ABS(RYM)) GO TO 17
          RYM = RY(I + 1, J + 1)
          IRYM = I
          JRYM = J
17        CONTINUE
        END DO
      END DO
      IF((M +(-1)) .LT. 0) GO TO 40
      IF((M +(-1)) .GT. 0) GO TO 25
      GO TO 13
13    CONTINUE
      doloop_ub3 = I2M
      DO I = I1P, doloop_ub3, 1
        RX(I + 1, 1) = (RX(I + 1, 1) / DD(I + 1, 1))
        RY(I + 1, 1) = (RY(I + 1, 1) / DD(I + 1, 1))
2       CONTINUE
      END DO
      GO TO 40
25    CONTINUE
      doloop_ub4 = I2M
      DO I = I1P, doloop_ub4, 1
        D(I + 1, 1) = (1D00 / DD(I + 1, 1))
4       CONTINUE
      END DO
      doloop_ub5 = M
      DO J = 2, doloop_ub5, 1
        doloop_ub6 = I2M
        DO I = I1P, doloop_ub6, 1
          R = (AA(I + 1, J + 1) * D(I + 1, J + (-1)))
          D(I + 1, J + 1) = (1D00 / ((DD(I + 1, J + 1) -(AA(I + 1, J +
     >  (-1)) * R))))
          RX(I + 1, J + 1) = (RX(I + 1, J + 1) -(RX(I + 1, J + (-1)) *
     >  R))
          RY(I + 1, J + 1) = (RY(I + 1, J + 1) -(RY(I + 1, J + (-1)) *
     >  R))
31        CONTINUE
        END DO
      END DO
      doloop_ub7 = I2M
      DO I = I1P, doloop_ub7, 1
        RX(I + 1, M + 1) = (D(I + 1, M + 1) * RX(I + 1, M + 1))
        RY(I + 1, M + 1) = (D(I + 1, M + 1) * RY(I + 1, M + 1))
34      CONTINUE
      END DO
      doloop_ub8 = M
      DO J = 2, doloop_ub8, 1
        K = ((M - J) + 1)
        doloop_ub9 = I2M
        DO I = I1P, doloop_ub9, 1
          RX(I + 1, K + 1) = (D(I + 1, K + 1) *((RX(I + 1, K + 1) -(AA(
     > I + 1, K + 1) * RX(I + 1, K + 1)))))
          RY(I + 1, K + 1) = (D(I + 1, K + 1) *((RY(I + 1, K + 1) -(AA(
     > I + 1, K + 1) * RY(I + 1, K + 1)))))
37        CONTINUE
        END DO
      END DO
      L = 0
      doloop_ub10 = J2M
      DO J = J1P, doloop_ub10, 1
        L = (L + 1)
        doloop_ub11 = I2M
        DO I = I1P, doloop_ub11, 1
          X(I + 1, J + 1) = (RX(I + 1, L + 1) + X(I + 1, J + 1))
          Y(I + 1, J + 1) = (RY(I + 1, L + 1) + Y(I + 1, J + 1))
20        CONTINUE
        END DO
      END DO
40    CONTINUE
      LL = (LL + 1)
      WRITE(6, '(I3,I4,I4,E11.4,I4,I4,E11.4,             I4,I4,E' //
     >  '11.4,I4,I4,E11.4)', IOSTAT = , REC = , SIZE = ) LL, IXCM, JXCM
     > , DXCM, IYCM, JYCM, DYCM, IRXM, JRXM, RXM, IRYM, JRYM, RYM
      ABX = ABS(RXM)
      ABY = ABS(RYM)
      DMAX = MAX(ABX, ABY)
      IF(LL .LT. LMAX .AND. DMAX .GT. EPS) GO TO 10
      STOP
      END
