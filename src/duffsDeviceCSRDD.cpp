#include "method.h"
#include <iostream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

///
/// DuffsDeviceCSRDD
///
DuffsDeviceCSRDD::DuffsDeviceCSRDD(int unrollingFactor) :
unrollingFactor(unrollingFactor) {
}

void DuffsDeviceCSRDD::convertMatrix() {
  int *rows = new int[csrMatrix->n];
  int *cols = csrMatrix->cols;
  double *vals = csrMatrix->vals;
  
#pragma omp parallel for
  for (int t = 0; t < stripeInfos->size(); ++t) {
    int i;
    for (i = stripeInfos->at(t).rowIndexBegin; i < stripeInfos->at(t).rowIndexEnd; i++) {
      int length = csrMatrix->rows[i + 1] - csrMatrix->rows[i];
      int entrancePoint = length % unrollingFactor;
      int iterations = length / unrollingFactor;
      rows[i] = (iterations << 8) | (entrancePoint & 0x000000FF);
    }
  }
  
  matrix = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
  matrix->numRows = csrMatrix->n;
  matrix->numCols = csrMatrix->nz;
  matrix->numVals = csrMatrix->nz;
}

DuffsDeviceCSRDD4::DuffsDeviceCSRDD4() :
DuffsDeviceCSRDD(4) {
  
}

DuffsDeviceCSRDD8::DuffsDeviceCSRDD8() :
DuffsDeviceCSRDD(8) {
  
}

DuffsDeviceCSRDD16::DuffsDeviceCSRDD16() :
DuffsDeviceCSRDD(16) {
  
}

DuffsDeviceCSRDD32::DuffsDeviceCSRDD32() :
DuffsDeviceCSRDD(32) {
  
}

void DuffsDeviceCSRDD4::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    int k = stripeInfos->at(t).valIndexBegin;
    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int ddInfo = rows[i];
      int n = ddInfo >> 8;
      const int entrancePoint = ddInfo & 0x000000FF;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 4; cols += 4;
            sum += vals[k-4] * v[cols[k-4]];
          case 3:
            sum += vals[k-3] * v[cols[k-3]];
          case 2:
            sum += vals[k-2] * v[cols[k-2]];
          case 1:
            sum += vals[k-1] * v[cols[k-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDeviceCSRDD8::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    int k = stripeInfos->at(t).valIndexBegin;
    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int ddInfo = rows[i];
      int n = ddInfo >> 8;
      const int entrancePoint = ddInfo & 0x000000FF;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 8; cols += 8;
            sum += vals[k-8] * v[cols[k-8]];
          case 7:
            sum += vals[k-7] * v[cols[k-7]];
          case 6:
            sum += vals[k-6] * v[cols[k-6]];
          case 5:
            sum += vals[k-5] * v[cols[k-5]];
          case 4:
            sum += vals[k-4] * v[cols[k-4]];
          case 3:
            sum += vals[k-3] * v[cols[k-3]];
          case 2:
            sum += vals[k-2] * v[cols[k-2]];
          case 1:
            sum += vals[k-1] * v[cols[k-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDeviceCSRDD16::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    int k = stripeInfos->at(t).valIndexBegin;
    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int ddInfo = rows[i];
      int n = ddInfo >> 8;
      const int entrancePoint = ddInfo & 0x000000FF;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 16; cols += 16;
            sum += vals[k-16] * v[cols[k-16]];
          case 15:
            sum += vals[k-15] * v[cols[k-15]];
          case 14:
            sum += vals[k-14] * v[cols[k-14]];
          case 13:
            sum += vals[k-13] * v[cols[k-13]];
          case 12:
            sum += vals[k-12] * v[cols[k-12]];
          case 11:
            sum += vals[k-11] * v[cols[k-11]];
          case 10:
            sum += vals[k-10] * v[cols[k-10]];
          case 9:
            sum += vals[k-9] * v[cols[k-9]];
          case 8:
            sum += vals[k-8] * v[cols[k-8]];
          case 7:
            sum += vals[k-7] * v[cols[k-7]];
          case 6:
            sum += vals[k-6] * v[cols[k-6]];
          case 5:
            sum += vals[k-5] * v[cols[k-5]];
          case 4:
            sum += vals[k-4] * v[cols[k-4]];
          case 3:
            sum += vals[k-3] * v[cols[k-3]];
          case 2:
            sum += vals[k-2] * v[cols[k-2]];
          case 1:
            sum += vals[k-1] * v[cols[k-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDeviceCSRDD32::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    int k = stripeInfos->at(t).valIndexBegin;
    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int ddInfo = rows[i];
      int n = ddInfo >> 8;
      const int entrancePoint = ddInfo & 0x000000FF;
      
      vals += entrancePoint;
      cols += entrancePoint;
      
      switch (entrancePoint) {
          do {
            vals += 32; cols += 32;
            sum += vals[k-32] * v[cols[k-32]];
          case 31:
            sum += vals[k-31] * v[cols[k-31]];
          case 30:
            sum += vals[k-30] * v[cols[k-30]];
          case 29:
            sum += vals[k-29] * v[cols[k-29]];
          case 28:
            sum += vals[k-28] * v[cols[k-28]];
          case 27:
            sum += vals[k-27] * v[cols[k-27]];
          case 26:
            sum += vals[k-26] * v[cols[k-26]];
          case 25:
            sum += vals[k-25] * v[cols[k-25]];
          case 24:
            sum += vals[k-24] * v[cols[k-24]];
          case 23:
            sum += vals[k-23] * v[cols[k-23]];
          case 22:
            sum += vals[k-22] * v[cols[k-22]];
          case 21:
            sum += vals[k-21] * v[cols[k-21]];
          case 20:
            sum += vals[k-20] * v[cols[k-20]];
          case 19:
            sum += vals[k-19] * v[cols[k-19]];
          case 18:
            sum += vals[k-18] * v[cols[k-18]];
          case 17:
            sum += vals[k-17] * v[cols[k-17]];
          case 16:
            sum += vals[k-16] * v[cols[k-16]];
          case 15:
            sum += vals[k-15] * v[cols[k-15]];
          case 14:
            sum += vals[k-14] * v[cols[k-14]];
          case 13:
            sum += vals[k-13] * v[cols[k-13]];
          case 12:
            sum += vals[k-12] * v[cols[k-12]];
          case 11:
            sum += vals[k-11] * v[cols[k-11]];
          case 10:
            sum += vals[k-10] * v[cols[k-10]];
          case 9:
            sum += vals[k-9] * v[cols[k-9]];
          case 8:
            sum += vals[k-8] * v[cols[k-8]];
          case 7:
            sum += vals[k-7] * v[cols[k-7]];
          case 6:
            sum += vals[k-6] * v[cols[k-6]];
          case 5:
            sum += vals[k-5] * v[cols[k-5]];
          case 4:
            sum += vals[k-4] * v[cols[k-4]];
          case 3:
            sum += vals[k-3] * v[cols[k-3]];
          case 2:
            sum += vals[k-2] * v[cols[k-2]];
          case 1:
            sum += vals[k-1] * v[cols[k-1]];
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}
