// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 15:28

#include "polynomial_fitting.h"

#include <third_party/eigen_3_3_7/include/Eigen/Dense>
using namespace Eigen;

#include "third_party/alglib/interpolation.h"
using namespace alglib;

//多项式拟合
int PolynomialFittingX(std::vector<yotta::Point> points, int nPower,
                       std::vector<double>& Coeff) {
  int nResult = 0;
  Coeff.clear();
  int nNum = points.size();
  if (nNum < MinNumber) {
    return 1;
  }
  if (nPower < 1) {
    return 1;
  }
  nPower++;

  double* pdX = new double[nNum]();
  double* pdY = new double[nNum]();
  for (int i = 0; i < nNum; i++) {
    pdX[i] = points[i].x;
    pdY[i] = points[i].y;
  }

  real_1d_array realX;
  real_1d_array realY;
  realX.setcontent(nNum, pdX);
  realY.setcontent(nNum, pdY);

  ae_int_t m = nPower;
  ae_int_t info;
  barycentricinterpolant p;
  polynomialfitreport rep;
  polynomialfit(realX, realY, nPower, info, p, rep);
  real_1d_array a2;
  polynomialbar2pow(p, a2);

  double* pdCoefficient = a2.getcontent();  //多项式系数
  for (int j = 0; j < nPower; j++) {
    Coeff.push_back(pdCoefficient[j]);
  }

  pdCoefficient = NULL;
  delete[] pdX;
  delete[] pdY;
  return nResult;
}

double Fx(double X, std::vector<double> Coeff) {
  double Y = 0;
  int nPower = Coeff.size();
  for (int j = 0; j < nPower; j++) {
    Y += Coeff[j] * pow(X, j);
  }
  return Y;
}

//多项式拟合
int PolynomialFittingXY(std::vector<yotta::Point3> points, int nPower,
                        std::vector<double>& Coeff) {
  Coeff.clear();
  if (nPower == 3) {
    return (PolynomialFittingXY_3(points, Coeff));
  }

  nPower++;
  int numberOfPara = nPower * (1 + nPower) / 2;
  // int n = points.size();

  MatrixXd A = MatrixXd::Constant(numberOfPara, numberOfPara, 0);  // A矩阵
  MatrixXd B = MatrixXd::Constant(numberOfPara, 1, 0);             // B矩阵
  for (int index = 0; index < points.size(); index++) {
    double x = points[index].x;
    double y = points[index].y;
    double z = points[index].z;

    double* X = new double[numberOfPara]();
    // 1 x y x^2 xy y^2 ....
    int ans = 0;
    for (int i = 0; i < nPower; i++) {
      for (int j = i; j >= 0; j--) {
        X[ans] = pow(x, j) * pow(y, i - j);
        ans++;
      }
    }

    for (int i = 0; i < numberOfPara; i++)  //行
    {
      for (int j = 0; j < numberOfPara; j++) {
        A(i, j) += X[j] * X[i];
      }
    }
    for (int i = 0; i < numberOfPara; i++) {
      B(i) += z * X[i];
    }

    delete[] X;
  }
  // AX = B
  // X = A^-1B
  MatrixXd Cof = A.inverse() * B;  //系数矩阵

  for (int i = 0; i < numberOfPara; i++) {
    Coeff.push_back(Cof(i));
  }

  return 0;
}

//少用循环
int PolynomialFittingXY_3(std::vector<yotta::Point3> points,
                          std::vector<double>& Coeff) {
  Coeff.clear();
  int nPower = 4;  //三次拟合
  int numberOfPara = nPower * (1 + nPower) / 2;

  double** PA = new double*[numberOfPara]();
  for (int i = 0; i < numberOfPara; i++) {
    PA[i] = new double[numberOfPara]();
  }

  double** PB = new double*[numberOfPara]();
  for (int i = 0; i < numberOfPara; i++) {
    PB[i] = new double[1]();
  }

  // int n = points.size();
  double x = 0.0, y = 0.0, z = 0.0;
  double x2 = 0.0, y2 = 0.0, xy = 0.0;
  double x3 = 0.0, y3 = 0.0, x2y = 0.0, xy2 = 0.0;

  //#pragma omp parallel for
  for (int indx = 0; indx < points.size(); indx++) {
    double x = points[indx].x;
    double y = points[indx].y;
    double z = points[indx].z;

    x2 = x * x;
    y2 = y * y;
    xy = x * y;

    x3 = x * x2;
    y3 = y * y2;
    x2y = x2 * y;
    xy2 = x * y2;

    /* first row */
    PA[0][0] += 1;
    PA[0][1] += x;
    PA[0][2] += y;
    PA[0][3] += x2;
    PA[0][4] += xy;
    PA[0][5] += y2;
    PA[0][6] += x3;
    PA[0][7] += x2y;
    PA[0][8] += xy2;
    PA[0][9] += y3;

    /* second row */
    PA[1][0] += x;
    PA[1][1] += x2;
    PA[1][2] += xy;
    PA[1][3] += x3;
    PA[1][4] += x2y;
    PA[1][5] += xy2;
    PA[1][6] += (x3 * x);
    PA[1][7] += (x2y * x);
    PA[1][8] += (xy2 * x);
    PA[1][9] += (y3 * x);

    /* third row */
    PA[2][0] += y;
    PA[2][1] += xy;
    PA[2][2] += y2;
    PA[2][3] += x2y;
    PA[2][4] += xy2;
    PA[2][5] += y3;
    PA[2][6] += (x3 * y);
    PA[2][7] += (x2y * y);
    PA[2][8] += (xy2 * y);
    PA[2][9] += (y3 * y);

    PA[3][0] += x2;
    PA[3][1] += x3;
    PA[3][2] += x2y;
    PA[3][3] += (x2 * x2);
    PA[3][4] += (xy * x2);
    PA[3][5] += (y2 * x2);
    PA[3][6] += (x3 * x2);
    PA[3][7] += (x2y * x2);
    PA[3][8] += (xy2 * x2);
    PA[3][9] += (y3 * x2);

    PA[4][0] += xy;
    PA[4][1] += (x * xy);
    PA[4][2] += (y * xy);
    PA[4][3] += (x2 * xy);
    PA[4][4] += (xy * xy);
    PA[4][5] += (y2 * xy);
    PA[4][6] += (x3 * xy);
    PA[4][7] += (x2y * xy);
    PA[4][8] += (xy2 * xy);
    PA[4][9] += (y3 * xy);

    PA[5][0] += y2;
    PA[5][1] += xy2;
    PA[5][2] += y3;
    PA[5][3] += (x2 * y2);
    PA[5][4] += (xy * y2);
    PA[5][5] += (y2 * y2);
    PA[5][6] += (x3 * y2);
    PA[5][7] += (x2y * y2);
    PA[5][8] += (xy2 * y2);
    PA[5][9] += (y3 * y2);

    PA[6][0] += x3;
    PA[6][1] += (x * x3);
    PA[6][2] += (y * x3);
    PA[6][3] += (x2 * x3);
    PA[6][4] += (xy * x3);
    PA[6][5] += (y2 * x3);
    PA[6][6] += (x3 * x3);
    PA[6][7] += (x2y * x3);
    PA[6][8] += (xy2 * x3);
    PA[6][9] += (y3 * x3);

    PA[7][0] += x2y;
    PA[7][1] += (x * x2y);
    PA[7][2] += (y * x2y);
    PA[7][3] += (x2 * x2y);
    PA[7][4] += (xy * x2y);
    PA[7][5] += (y2 * x2y);
    PA[7][6] += (x3 * x2y);
    PA[7][7] += (x2y * x2y);
    PA[7][8] += (xy2 * x2y);
    PA[7][9] += (y3 * x2y);

    PA[8][0] += xy2;
    PA[8][1] += (x * xy2);
    PA[8][2] += (y * xy2);
    PA[8][3] += (x2 * xy2);
    PA[8][4] += (xy * xy2);
    PA[8][5] += (y2 * xy2);
    PA[8][6] += (x3 * xy2);
    PA[8][7] += (x2y * xy2);
    PA[8][8] += (xy2 * xy2);
    PA[8][9] += (y3 * xy2);

    PA[9][0] += y3;
    PA[9][1] += (x * y3);
    PA[9][2] += (y * y3);
    PA[9][3] += (x2 * y3);
    PA[9][4] += (xy * y3);
    PA[9][5] += (y2 * y3);
    PA[9][6] += (x3 * y3);
    PA[9][7] += (x2y * y3);
    PA[9][8] += (xy2 * y3);
    PA[9][9] += (y3 * y3);

    /* matrix B */
    PB[0][0] += z;
    PB[1][0] += (x * z);
    PB[2][0] += (y * z);
    PB[3][0] += (x2 * z);
    PB[4][0] += (xy * z);
    PB[5][0] += (y2 * z);
    PB[6][0] += (x3 * z);
    PB[7][0] += (x2y * z);
    PB[8][0] += (xy2 * z);
    PB[9][0] += (y3 * z);

    // delete[] X;
  }

  MatrixXd A = MatrixXd::Constant(numberOfPara, numberOfPara, 0);  // A矩阵
  MatrixXd B = MatrixXd::Constant(numberOfPara, 1, 0);             // B矩阵

  for (int i = 0; i < numberOfPara; i++)  //行
  {
    for (int j = 0; j < numberOfPara; j++) {
      A(i, j) = PA[i][j];
    }
  }
  for (int i = 0; i < numberOfPara; i++) {
    B(i) = PB[i][0];
  }

  MatrixXd Cof = A.inverse() * B;  //系数矩阵
  Coeff.clear();
  for (int i = 0; i < numberOfPara; i++) {
    Coeff.push_back(Cof(i));
  }

  /* release memory */
  for (int i = 0; i < numberOfPara; i++) {
    delete[] PA[i];
    delete[] PB[i];
  }
  delete[] PA;
  delete[] PB;

  return 0;
}

double Fxy(double X, double Y, int nPower, std::vector<double> Coeff) {
  double Z = 0;
  int ans = 0;
  for (int i = 0; i < (nPower + 1); i++) {
    for (int j = i; j >= 0; j--) {
      Z += pow(X, j) * pow(Y, i - j) * Coeff[ans++];
    }
  }
  return Z;
}
