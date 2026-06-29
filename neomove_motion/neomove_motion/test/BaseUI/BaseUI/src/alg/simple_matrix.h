// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 15:28

#ifndef PROBE_STATION8_SRC_ALG_SIMPLE_MATRIX_H_
#define PROBE_STATION8_SRC_ALG_SIMPLE_MATRIX_H_

#include <stdio.h>
#include <stdlib.h>

constexpr double kUninitializedValue = -31415.0;

class SimpleMatrix {
 public:
  SimpleMatrix(int mm = 0, int nn = 0);

  void SetM(int mm);
  void SetN(int nn);
  void InitMatrix();
  void FreeMatrix();
  double Read(int i, int j);
  int Write(int i, int j, double val);

  int m;
  int n;
  double* arr = nullptr;
};

class SimpleMatrixCalc {
 public:
  SimpleMatrixCalc();

  int Add(SimpleMatrix* A, SimpleMatrix* B, SimpleMatrix* C);
  int Subtract(SimpleMatrix* A, SimpleMatrix* B, SimpleMatrix* C);
  int Multiply(SimpleMatrix* A, SimpleMatrix* B, SimpleMatrix* C);
  double Det(SimpleMatrix* A);
  int Transpose(SimpleMatrix* A, SimpleMatrix* B);
  int Inverse(SimpleMatrix* A, SimpleMatrix* B);
};

void PrintMatrix(SimpleMatrix* A);

#endif  // PROBE_STATION8_SRC_ALG_SIMPLE_MATRIX_H_
