// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 15:28

#ifndef PROBE_STATION8_SRC_ALG_POLYNOMIAL_FITTING_H_
#define PROBE_STATION8_SRC_ALG_POLYNOMIAL_FITTING_H_

#include <vector>

#include "base.h"
//实现以下两种形式的多项式函数拟合
// Y = f(X)
// Z = f(X,Y)

#define MinNumber 5  //拟合需要最小点数

//多项式拟合y = a0 + a1 * x + ... + an * x^n;
int PolynomialFittingX(std::vector<yotta::Point> points, int nPower,
                       std::vector<double> &Coeff);
double Fx(double X, std::vector<double> Coeff);

//多项式拟合z = a1 + a2*x + a3*y + a4*x^2 + a5*x*y + a6*y^2 + a7*x^3 + a8*x^2*y
//+ a9*x*y^2 + a10*y^3 +...... 多项式系数个数 = sum(1 : 最高次+1) = N * (1+N) /
// 2
// N = 最高次次数 + 1
/*
        0
        X      Y
        XX    XY  YY
        XXX    XXY  XYY YYY
        XXXX  XXXY  XYYY  XXYY  YYYY
        XXXXX  XXXXY XXXYY  XXYYY  XYYYY YYYYY
        ...
*/
int PolynomialFittingXY(std::vector<yotta::Point3> points, int nPower,
                        std::vector<double> &Coeff);

// 3次多项式示例
// z = a1 + a2*x + a3*y + a4*x^2 + a5*x*y + a6*y^2 + a7*x^3 + a8*y*x^2 +
// a9*x*y^2 + a10*y^3
int PolynomialFittingXY_3(std::vector<yotta::Point3> points,
                          std::vector<double> &Coeff);  //仅限三次拟合速度更快

double Fxy(double X, double Y, int nPower, std::vector<double> Coeff);

#endif  // PROBE_STATION8_SRC_ALG_POLYNOMIAL_FITTING_H_
