#include "method.h"

using namespace thundercat;
using namespace std;

void PlainCSR::spmv(double* __restrict v, double* __restrict w) {
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

void PlainCSR4::spmv(double* __restrict v, double* __restrict w) {
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

void PlainCSR8::spmv(double* __restrict v, double* __restrict w) {
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

void PlainCSR16::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = matrix->rows[i]; k < matrix->rows[i + 1] - 15; k += 16) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
        ww += matrix->vals[k+1] * v[matrix->cols[k+1]];
        ww += matrix->vals[k+2] * v[matrix->cols[k+2]];
        ww += matrix->vals[k+3] * v[matrix->cols[k+3]];
        ww += matrix->vals[k+4] * v[matrix->cols[k+4]];
        ww += matrix->vals[k+5] * v[matrix->cols[k+5]];
        ww += matrix->vals[k+6] * v[matrix->cols[k+6]];
        ww += matrix->vals[k+7] * v[matrix->cols[k+7]];
        ww += matrix->vals[k+8] * v[matrix->cols[k+8]];
        ww += matrix->vals[k+9] * v[matrix->cols[k+9]];
        ww += matrix->vals[k+10] * v[matrix->cols[k+10]];
        ww += matrix->vals[k+11] * v[matrix->cols[k+11]];
        ww += matrix->vals[k+12] * v[matrix->cols[k+12]];
        ww += matrix->vals[k+13] * v[matrix->cols[k+13]];
        ww += matrix->vals[k+14] * v[matrix->cols[k+14]];
        ww += matrix->vals[k+15] * v[matrix->cols[k+15]];
      }
      for (; k < matrix->rows[i + 1]; k++) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
      }
      w[i] += ww;
    }
  }
}

void PlainCSR32::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = matrix->rows[i]; k < matrix->rows[i + 1] - 31; k += 32) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
        ww += matrix->vals[k+1] * v[matrix->cols[k+1]];
        ww += matrix->vals[k+2] * v[matrix->cols[k+2]];
        ww += matrix->vals[k+3] * v[matrix->cols[k+3]];
        ww += matrix->vals[k+4] * v[matrix->cols[k+4]];
        ww += matrix->vals[k+5] * v[matrix->cols[k+5]];
        ww += matrix->vals[k+6] * v[matrix->cols[k+6]];
        ww += matrix->vals[k+7] * v[matrix->cols[k+7]];
        ww += matrix->vals[k+8] * v[matrix->cols[k+8]];
        ww += matrix->vals[k+9] * v[matrix->cols[k+9]];
        ww += matrix->vals[k+10] * v[matrix->cols[k+10]];
        ww += matrix->vals[k+11] * v[matrix->cols[k+11]];
        ww += matrix->vals[k+12] * v[matrix->cols[k+12]];
        ww += matrix->vals[k+13] * v[matrix->cols[k+13]];
        ww += matrix->vals[k+14] * v[matrix->cols[k+14]];
        ww += matrix->vals[k+15] * v[matrix->cols[k+15]];
        ww += matrix->vals[k+16] * v[matrix->cols[k+16]];
        ww += matrix->vals[k+17] * v[matrix->cols[k+17]];
        ww += matrix->vals[k+18] * v[matrix->cols[k+18]];
        ww += matrix->vals[k+19] * v[matrix->cols[k+19]];
        ww += matrix->vals[k+20] * v[matrix->cols[k+20]];
        ww += matrix->vals[k+21] * v[matrix->cols[k+21]];
        ww += matrix->vals[k+22] * v[matrix->cols[k+22]];
        ww += matrix->vals[k+23] * v[matrix->cols[k+23]];
        ww += matrix->vals[k+24] * v[matrix->cols[k+24]];
        ww += matrix->vals[k+25] * v[matrix->cols[k+25]];
        ww += matrix->vals[k+26] * v[matrix->cols[k+26]];
        ww += matrix->vals[k+27] * v[matrix->cols[k+27]];
        ww += matrix->vals[k+28] * v[matrix->cols[k+28]];
        ww += matrix->vals[k+29] * v[matrix->cols[k+29]];
        ww += matrix->vals[k+30] * v[matrix->cols[k+30]];
        ww += matrix->vals[k+31] * v[matrix->cols[k+31]];
      }
      for (; k < matrix->rows[i + 1]; k++) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
      }
      w[i] += ww;
    }
  }
}
