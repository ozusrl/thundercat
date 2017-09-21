#pragma once

#include "method.h"
#include <iostream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

///
/// Duff's Device with compression
///
template <unsigned int UnrollingFactor>
class DuffsDeviceCompressed: public SpMVMethod {
public:
  DuffsDeviceCompressed();
  
  virtual void spmv(double* __restrict v, double* __restrict w) final;

protected:
  virtual void convertMatrix() final;
  
private:
  template <typename T>
  void spmvDD(double* __restrict v, double* __restrict w, T* __restrict rows);
  
protected:
  int sizeOfRowLength;
};

template <unsigned int UnrollingFactor>
DuffsDeviceCompressed<UnrollingFactor>::DuffsDeviceCompressed() {
}

template <unsigned int UnrollingFactor>
void DuffsDeviceCompressed<UnrollingFactor>::convertMatrix() {
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

template <unsigned int UnrollingFactor>
void DuffsDeviceCompressed<UnrollingFactor>::spmv(double* __restrict v, double* __restrict w) {
  const int *rows = matrix->rows;
  switch(sizeOfRowLength) {
  case 1:
    spmvDD(v, w, (unsigned char *)rows); break;
  case 2:
    spmvDD(v, w, (unsigned short *)rows); break;
  default:
    spmvDD(v, w, (unsigned int *)rows); break;
  }
}

template <>
template <typename T>
void DuffsDeviceCompressed<4>::spmvDD(double* __restrict v, double* __restrict w, T* __restrict rows) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    rows += stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const T length = rows[i];
      int n = (int)(length / 4);
      const T entrancePoint = length % 4;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 4; cols += 4;
            sum += vals[-4] * v[cols[-4]];
          case 3:
            sum += vals[-3] * v[cols[-3]];
          case 2:
            sum += vals[-2] * v[cols[-2]];
          case 1:
            sum += vals[-1] * v[cols[-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

template <>
template <typename T>
void DuffsDeviceCompressed<8>::spmvDD(double* __restrict v, double* __restrict w, T* __restrict rows) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    rows += stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const T length = rows[i];
      int n = (int)(length / 8);
      const T entrancePoint = length % 8;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 8; cols += 8;
            sum += vals[-8] * v[cols[-8]];
          case 7:
            sum += vals[-7] * v[cols[-7]];
          case 6:
            sum += vals[-6] * v[cols[-6]];
          case 5:
            sum += vals[-5] * v[cols[-5]];
          case 4:
            sum += vals[-4] * v[cols[-4]];
          case 3:
            sum += vals[-3] * v[cols[-3]];
          case 2:
            sum += vals[-2] * v[cols[-2]];
          case 1:
            sum += vals[-1] * v[cols[-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

template <>
template <typename T>
void DuffsDeviceCompressed<16>::spmvDD(double* __restrict v, double* __restrict w, T* __restrict rows) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    rows += stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const T length = rows[i];
      int n = (int)(length / 16);
      const T entrancePoint = length % 16;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 16; cols += 16;
            sum += vals[-16] * v[cols[-16]];
          case 15:
            sum += vals[-15] * v[cols[-15]];
          case 14:
            sum += vals[-14] * v[cols[-14]];
          case 13:
            sum += vals[-13] * v[cols[-13]];
          case 12:
            sum += vals[-12] * v[cols[-12]];
          case 11:
            sum += vals[-11] * v[cols[-11]];
          case 10:
            sum += vals[-10] * v[cols[-10]];
          case 9:
            sum += vals[-9] * v[cols[-9]];
          case 8:
            sum += vals[-8] * v[cols[-8]];
          case 7:
            sum += vals[-7] * v[cols[-7]];
          case 6:
            sum += vals[-6] * v[cols[-6]];
          case 5:
            sum += vals[-5] * v[cols[-5]];
          case 4:
            sum += vals[-4] * v[cols[-4]];
          case 3:
            sum += vals[-3] * v[cols[-3]];
          case 2:
            sum += vals[-2] * v[cols[-2]];
          case 1:
            sum += vals[-1] * v[cols[-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

template <>
template <typename T>
void DuffsDeviceCompressed<32>::spmvDD(double* __restrict v, double* __restrict w, T* __restrict rows) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    rows += stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const T length = rows[i];
      int n = (int)(length / 32);
      const T entrancePoint = length % 32;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 32; cols += 32;
            sum += vals[-32] * v[cols[-32]];
          case 31:
            sum += vals[-31] * v[cols[-31]];
          case 30:
            sum += vals[-30] * v[cols[-30]];
          case 29:
            sum += vals[-29] * v[cols[-29]];
          case 28:
            sum += vals[-28] * v[cols[-28]];
          case 27:
            sum += vals[-27] * v[cols[-27]];
          case 26:
            sum += vals[-26] * v[cols[-26]];
          case 25:
            sum += vals[-25] * v[cols[-25]];
          case 24:
            sum += vals[-24] * v[cols[-24]];
          case 23:
            sum += vals[-23] * v[cols[-23]];
          case 22:
            sum += vals[-22] * v[cols[-22]];
          case 21:
            sum += vals[-21] * v[cols[-21]];
          case 20:
            sum += vals[-20] * v[cols[-20]];
          case 19:
            sum += vals[-19] * v[cols[-19]];
          case 18:
            sum += vals[-18] * v[cols[-18]];
          case 17:
            sum += vals[-17] * v[cols[-17]];
          case 16:
            sum += vals[-16] * v[cols[-16]];
          case 15:
            sum += vals[-15] * v[cols[-15]];
          case 14:
            sum += vals[-14] * v[cols[-14]];
          case 13:
            sum += vals[-13] * v[cols[-13]];
          case 12:
            sum += vals[-12] * v[cols[-12]];
          case 11:
            sum += vals[-11] * v[cols[-11]];
          case 10:
            sum += vals[-10] * v[cols[-10]];
          case 9:
            sum += vals[-9] * v[cols[-9]];
          case 8:
            sum += vals[-8] * v[cols[-8]];
          case 7:
            sum += vals[-7] * v[cols[-7]];
          case 6:
            sum += vals[-6] * v[cols[-6]];
          case 5:
            sum += vals[-5] * v[cols[-5]];
          case 4:
            sum += vals[-4] * v[cols[-4]];
          case 3:
            sum += vals[-3] * v[cols[-3]];
          case 2:
            sum += vals[-2] * v[cols[-2]];
          case 1:
            sum += vals[-1] * v[cols[-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}
