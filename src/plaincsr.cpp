#include "method.h"
#ifdef OMP_EXISTS
#include "omp.h"
#endif

using namespace thundercat;
using namespace std;

void PlainCSR::spmv(double *v, double *w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      for (int k = matrix->rows[i]; k < matrix->rows[i + 1]; k++) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
      }
      w[i] += ww;
    }
  }
}

void PlainCSR2::spmv(double *v, double *w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = matrix->rows[i]; k < matrix->rows[i + 1] - 1; k += 2) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
        ww += matrix->vals[k+1] * v[matrix->cols[k+1]];
      }
      for (; k < matrix->rows[i + 1]; k++) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
      }
      w[i] += ww;
    }
  }
}

void PlainCSR4::spmv(double *v, double *w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = matrix->rows[i]; k < matrix->rows[i + 1] - 3; k += 4) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
        ww += matrix->vals[k+1] * v[matrix->cols[k+1]];
        ww += matrix->vals[k+2] * v[matrix->cols[k+2]];
        ww += matrix->vals[k+3] * v[matrix->cols[k+3]];
      }
      for (; k < matrix->rows[i + 1]; k++) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
      }
      w[i] += ww;
    }
  }
}

void PlainCSR8::spmv(double *v, double *w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = matrix->rows[i]; k < matrix->rows[i + 1] - 7; k += 8) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
        ww += matrix->vals[k+1] * v[matrix->cols[k+1]];
        ww += matrix->vals[k+2] * v[matrix->cols[k+2]];
        ww += matrix->vals[k+3] * v[matrix->cols[k+3]];
        ww += matrix->vals[k+4] * v[matrix->cols[k+4]];
        ww += matrix->vals[k+5] * v[matrix->cols[k+5]];
        ww += matrix->vals[k+6] * v[matrix->cols[k+6]];
        ww += matrix->vals[k+7] * v[matrix->cols[k+7]];
      }
      for (; k < matrix->rows[i + 1]; k++) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
      }
      w[i] += ww;
    }
  }
}
