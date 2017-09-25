#pragma once

#include "method.h"
#include <iostream>

using namespace thundercat;
using namespace std;

///
/// Incremental rows array with compression
///
class RowIncrementalCSR: public SpMVMethod {
public:
  RowIncrementalCSR();
  
  virtual void spmv(double* __restrict v, double* __restrict w) final;

protected:
  virtual void convertMatrix() final;
  
private:
  template <typename T>
  void spmvICSR(double* __restrict v, double* __restrict w, T* __restrict rows);
  
protected:
  int sizeOfRowLength;
};

RowIncrementalCSR::RowIncrementalCSR() {
}

void RowIncrementalCSR::convertMatrix() {
  int *rows = new int[csrMatrix->n];
  unsigned char *rowPtr = (unsigned char *)rows;
  int *cols = csrMatrix->cols;
  double *vals = csrMatrix->vals;

  int maxRowLength = 0;
  for (int i = 0; i < csrMatrix->n; i++) {
    int length = csrMatrix->rows[i + 1] - csrMatrix->rows[i];
    if (length > maxRowLength) {
      maxRowLength = length;
    }
  }
  
  if (maxRowLength <= 256) {             // 1 byte
    sizeOfRowLength = 1;
  } else if (maxRowLength <= 65536) {    // 2 bytes
    sizeOfRowLength = 2;
  } else {                               // 4 bytes
    sizeOfRowLength = 4;
  }
  
#pragma omp parallel for
  for (int t = 0; t < stripeInfos->size(); ++t) {
    int i;
    for (i = stripeInfos->at(t).rowIndexBegin; i < stripeInfos->at(t).rowIndexEnd; i++) {
      int length = csrMatrix->rows[i + 1] - csrMatrix->rows[i];
      unsigned char *charPtr = (unsigned char*)rowPtr;
      unsigned short *shortPtr = (unsigned short*)rowPtr;
      int *intPtr = (int*)rowPtr;
      switch(sizeOfRowLength) {
      case 1:
        *charPtr = (unsigned char)length; break;
      case 2:
        *shortPtr = (unsigned short)length; break;
      default:
        *intPtr = (int)length; break;
      }
      rowPtr += sizeOfRowLength;
    }
  }
  
  matrix = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->m, csrMatrix->nz);
  matrix->numRows = csrMatrix->n;
  matrix->numCols = csrMatrix->nz;
  matrix->numVals = csrMatrix->nz;
}

void RowIncrementalCSR::spmv(double* __restrict v, double* __restrict w) {
  const int *rows = matrix->rows;
  switch(sizeOfRowLength) {
  case 1:
    spmvICSR(v, w, (unsigned char *)rows); break;
  case 2:
    spmvICSR(v, w, (unsigned short *)rows); break;
  default:
    spmvICSR(v, w, (unsigned int *)rows); break;
  }
}

template <typename T>
void RowIncrementalCSR::spmvICSR(double* __restrict v, double* __restrict w, T* __restrict rows) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    rows += stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
  
    int k = stripeInfos->at(t).valIndexBegin;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const T length = rows[i];
      for (T j = 0; j < length; j++) {
        sum += matrix->vals[k] * v[matrix->cols[k]];
        k++;
      }
      w[i] += sum;
    }
  }
}

