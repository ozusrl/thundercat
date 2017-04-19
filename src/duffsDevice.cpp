#include "method.h"

using namespace thundercat;
using namespace std;

void DuffsDevice4::spmv(double *v, double *w) {
  const int M = 4;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      int k = rows[i];
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      int currentCase = length % M;
      switch (currentCase) {
          do {
            sum += vals[k] * v[cols[k]]; k++;
          case 3:
            sum += vals[k] * v[cols[k]]; k++;
          case 2:
            sum += vals[k] * v[cols[k]]; k++;
          case 1:
            sum += vals[k] * v[cols[k]]; k++;
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDevice6::spmv(double *v, double *w) {
  const int M = 6;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      int k = rows[i];
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      int currentCase = length % M;
      switch (currentCase) {
          do {
            sum += vals[k] * v[cols[k]]; k++;
          case 5:
            sum += vals[k] * v[cols[k]]; k++;
          case 4:
            sum += vals[k] * v[cols[k]]; k++;
          case 3:
            sum += vals[k] * v[cols[k]]; k++;
          case 2:
            sum += vals[k] * v[cols[k]]; k++;
          case 1:
            sum += vals[k] * v[cols[k]]; k++;
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDevice8::spmv(double *v, double *w) {
  const int M = 8;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      int k = rows[i];
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      int currentCase = length % M;
      switch (currentCase) {
          do {
            sum += vals[k] * v[cols[k]]; k++;
          case 7:
            sum += vals[k] * v[cols[k]]; k++;
          case 6:
            sum += vals[k] * v[cols[k]]; k++;
          case 5:
            sum += vals[k] * v[cols[k]]; k++;
          case 4:
            sum += vals[k] * v[cols[k]]; k++;
          case 3:
            sum += vals[k] * v[cols[k]]; k++;
          case 2:
            sum += vals[k] * v[cols[k]]; k++;
          case 1:
            sum += vals[k] * v[cols[k]]; k++;
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDevice12::spmv(double *v, double *w) {
  const int M = 12;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      int k = rows[i];
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      int currentCase = length % M;
      switch (currentCase) {
          do {
            sum += vals[k] * v[cols[k]]; k++;
          case 11:
            sum += vals[k] * v[cols[k]]; k++;
          case 10:
            sum += vals[k] * v[cols[k]]; k++;
          case 9:
            sum += vals[k] * v[cols[k]]; k++;
          case 8:
            sum += vals[k] * v[cols[k]]; k++;
          case 7:
            sum += vals[k] * v[cols[k]]; k++;
          case 6:
            sum += vals[k] * v[cols[k]]; k++;
          case 5:
            sum += vals[k] * v[cols[k]]; k++;
          case 4:
            sum += vals[k] * v[cols[k]]; k++;
          case 3:
            sum += vals[k] * v[cols[k]]; k++;
          case 2:
            sum += vals[k] * v[cols[k]]; k++;
          case 1:
            sum += vals[k] * v[cols[k]]; k++;
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}
