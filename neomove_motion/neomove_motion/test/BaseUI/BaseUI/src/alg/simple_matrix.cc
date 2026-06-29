// copyright 2025 YottaImage. All rights reserved.
// author zhangshuangcheng
// date 2026/03/30 15:28

#include "simple_matrix.h"

SimpleMatrix::SimpleMatrix(int mm, int nn) {
  m = mm;
  n = nn;
}

void SimpleMatrix::SetM(int mm) { m = mm; }

void SimpleMatrix::SetN(int nn) { n = nn; }

void SimpleMatrix::InitMatrix() { arr = new double[m * n]; }

void SimpleMatrix::FreeMatrix() {
  delete[] arr;
  arr = nullptr;
}

double SimpleMatrix::Read(int i, int j) {
  if (i >= m || j >= n) {
    return -1;
  }

  return *(arr + i * n + j);
}

int SimpleMatrix::Write(int i, int j, double val) {
  if (i >= m || j >= n) {
    return -1;
  }

  *(arr + i * n + j) = val;
  return 1;
}

SimpleMatrixCalc::SimpleMatrixCalc() {}

int SimpleMatrixCalc::Add(SimpleMatrix* A, SimpleMatrix* B, SimpleMatrix* C) {
  if (A->m != B->m || A->n != B->n || A->m != C->m || A->n != C->n) {
    return -1;
  }

  for (int i = 0; i < C->m; i++) {
    for (int j = 0; j < C->n; j++) {
      C->Write(i, j, A->Read(i, j) + B->Read(i, j));
    }
  }

  return 1;
}

int SimpleMatrixCalc::Subtract(SimpleMatrix* A, SimpleMatrix* B,
                               SimpleMatrix* C) {
  if (A->m != B->m || A->n != B->n || A->m != C->m || A->n != C->n) {
    return -1;
  }

  for (int i = 0; i < C->m; i++) {
    for (int j = 0; j < C->n; j++) {
      C->Write(i, j, A->Read(i, j) - B->Read(i, j));
    }
  }

  return 1;
}

int SimpleMatrixCalc::Multiply(SimpleMatrix* A, SimpleMatrix* B,
                               SimpleMatrix* C) {
  if (A->m != C->m || B->n != C->n || A->n != B->m) {
    return -1;
  }

  for (int i = 0; i < C->m; i++) {
    for (int j = 0; j < C->n; j++) {
      double temp = 0;
      for (int k = 0; k < A->n; k++) {
        temp += A->Read(i, k) * B->Read(k, j);
      }
      C->Write(i, j, temp);
    }
  }

  return 1;
}

double SimpleMatrixCalc::Det(SimpleMatrix* A) {
  if (A->m != A->n || (A->m != 2 && A->m != 3)) {
    return kUninitializedValue;
  }

  if (A->m == 2) {
    return A->Read(0, 0) * A->Read(1, 1) - A->Read(0, 1) * A->Read(1, 0);
  }

  return A->Read(0, 0) * A->Read(1, 1) * A->Read(2, 2) +
         A->Read(0, 1) * A->Read(1, 2) * A->Read(2, 0) +
         A->Read(0, 2) * A->Read(1, 0) * A->Read(2, 1) -
         A->Read(0, 0) * A->Read(1, 2) * A->Read(2, 1) -
         A->Read(0, 1) * A->Read(1, 0) * A->Read(2, 2) -
         A->Read(0, 2) * A->Read(1, 1) * A->Read(2, 0);
}

int SimpleMatrixCalc::Transpose(SimpleMatrix* A, SimpleMatrix* B) {
  if (A->m != B->n || A->n != B->m) {
    return -1;
  }

  for (int i = 0; i < B->m; i++) {
    for (int j = 0; j < B->n; j++) {
      B->Write(i, j, A->Read(j, i));
    }
  }

  return 1;
}

void PrintMatrix(SimpleMatrix* A) {
  for (int i = 0; i < A->m; i++) {
    for (int j = 0; j < A->n; j++) {
      printf("%f ", A->Read(i, j));
    }
    printf("\n");
  }
  printf("\n\n");
}

int SimpleMatrixCalc::Inverse(SimpleMatrix* A, SimpleMatrix* B) {
  if (A->m != A->n || B->m != B->n || A->m != B->m) {
    return -1;
  }

  SimpleMatrix m(A->m, 2 * A->m);
  m.InitMatrix();

  for (int i = 0; i < m.m; i++) {
    for (int j = 0; j < m.n; j++) {
      if (j <= A->n - 1) {
        m.Write(i, j, A->Read(i, j));
      } else if (i == j - A->n) {
        m.Write(i, j, 1);
      } else {
        m.Write(i, j, 0);
      }
    }
  }

  for (int k = 0; k < m.m - 1; k++) {
    if (m.Read(k, k) == 0) {
      int swap_row = k + 1;
      for (; swap_row < m.m; swap_row++) {
        if (m.Read(swap_row, k) != 0) {
          break;
        }
      }
      if (swap_row >= m.m) {
        m.FreeMatrix();
        return -1;
      }
      for (int j = 0; j < m.n; j++) {
        double temp = m.Read(k, j);
        m.Write(k, j, m.Read(swap_row, j));
        m.Write(swap_row, j, temp);
      }
    }

    for (int i = k + 1; i < m.m; i++) {
      double factor = m.Read(i, k) / m.Read(k, k);
      for (int j = 0; j < m.n; j++) {
        double temp = m.Read(i, j) - factor * m.Read(k, j);
        m.Write(i, j, temp);
      }
    }
  }

  for (int k = m.m - 1; k > 0; k--) {
    if (m.Read(k, k) == 0) {
      m.FreeMatrix();
      return -1;
    }

    for (int i = k - 1; i >= 0; i--) {
      double factor = m.Read(i, k) / m.Read(k, k);
      for (int j = 0; j < m.n; j++) {
        double temp = m.Read(i, j) - factor * m.Read(k, j);
        m.Write(i, j, temp);
      }
    }
  }

  for (int i = 0; i < m.m; i++) {
    if (m.Read(i, i) == 0) {
      m.FreeMatrix();
      return -1;
    }

    if (m.Read(i, i) != 1) {
      double factor = 1 / m.Read(i, i);
      for (int j = 0; j < m.n; j++) {
        m.Write(i, j, m.Read(i, j) * factor);
      }
    }
  }

  for (int i = 0; i < B->m; i++) {
    for (int j = 0; j < B->m; j++) {
      B->Write(i, j, m.Read(i, j + m.m));
    }
  }

  m.FreeMatrix();
  return 1;
}
