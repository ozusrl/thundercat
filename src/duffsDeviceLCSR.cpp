#include "method.h"
#include <iostream>

using namespace thundercat;
using namespace std;
using namespace asmjit;
using namespace x86;

///
/// Analysis
///
void DuffsDeviceLCSR::analyzeMatrix() {
  LCSRAnalyzer analyzer;
  analyzer.analyzeMatrix(csrMatrix, stripeInfos, rowByNZLists);
}

///
/// DuffsDeviceLCSR
///
void DuffsDeviceLCSR::convertMatrix() {
  int *rows = new int[csrMatrix->n];
  int *cols = new int[csrMatrix->nz];
  double *vals = new double[csrMatrix->nz];
  
  // TODO: Fix this for multi-threading
  int numLengths = rowByNZLists.at(0).size();
  int *length = new int[numLengths];
  int *lenStart = new int[numLengths + 1];
  int *lengthPtr = length;
  int *lenStartPtr = lenStart;
  *lenStartPtr = 0;
  
#pragma omp parallel for
  for (int t = 0; t < rowByNZLists.size(); ++t) {
    auto &rowByNZList = rowByNZLists.at(t);
    int *rowsPtr = rows + stripeInfos->at(t).rowIndexBegin;
    int *colsPtr = cols + stripeInfos->at(t).valIndexBegin;
    double *valsPtr = vals + stripeInfos->at(t).valIndexBegin;
    
    for (auto &rowByNZ : rowByNZList) {
      unsigned long rowLength = rowByNZ.first;
      *lengthPtr++ = rowLength;
      int index = *lenStartPtr + rowByNZ.second.getRowIndices()->size();
      lenStartPtr++;
      *lenStartPtr = index;
      
      for (int rowIndex : *(rowByNZ.second.getRowIndices())) {
        *rowsPtr++ = rowIndex;
        int k = csrMatrix->rows[rowIndex];
        for (int i = 0; i < rowLength; i++, k++) {
          *colsPtr++ = csrMatrix->cols[k];
          *valsPtr++ = csrMatrix->vals[k];
        }
      }
    }
  }
  
  matrix = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->nz);
  lcsrInfo = new LCSRInfo(numLengths, length, lenStart);
}

void DuffsDeviceLCSR4::spmv(double* __restrict v, double* __restrict w) {
  const int M = 4;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
  
  // TODO: Fix this for multi-threading
  int k = 0;
  for (int i = 0; i < lcsrInfo->numLengths; i++) {
    int length = lcsrInfo->length[i];
    if (length == 0) break;
    
    int lenEnd = lcsrInfo->lenStart[i + 1];
    int enter = length % M;
    const int iterations = (enter > 0) + (length / M);
    
    for (int r = lcsrInfo->lenStart[i]; r < lenEnd; r++) {
      int rowIndex = rows[r];
      double sum = 0.0;
      int n = iterations;
      
      vals += enter;
      cols += enter;
      
      switch (enter) {
        case 0: do {
          vals += 4; cols += 4;
          sum += vals[k-4] * v[cols[k-4]];
        case 3:
          sum += vals[k-3] * v[cols[k-3]];
        case 2:
          sum += vals[k-2] * v[cols[k-2]];
        case 1:
          sum += vals[k-1] * v[cols[k-1]];
        } while (--n > 0);
      }
      w[rowIndex] += sum;
    }
  }
}

void DuffsDeviceLCSR8::spmv(double* __restrict v, double* __restrict w) {
  const int M = 8;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;

  // TODO: Fix this for multi-threading
  int k = 0;
  for (int i = 0; i < lcsrInfo->numLengths; i++) {
    int length = lcsrInfo->length[i];
    if (length == 0) break;
    
    int lenEnd = lcsrInfo->lenStart[i + 1];
    int enter = length % M;
    const int iterations = (enter > 0) + (length / M);
    
    for (int r = lcsrInfo->lenStart[i]; r < lenEnd; r++) {
      int rowIndex = rows[r];
      double sum = 0.0;
      int n = iterations;

      vals += enter;
      cols += enter;
      
      switch (enter) {
        case 0: do {
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
        } while (--n > 0);
      }
      w[rowIndex] += sum;
    }
  }
}

void DuffsDeviceLCSR16::spmv(double* __restrict v, double* __restrict w) {
  const int M = 16;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
  
  // TODO: Fix this for multi-threading
  int k = 0;
  for (int i = 0; i < lcsrInfo->numLengths; i++) {
    int length = lcsrInfo->length[i];
    if (length == 0) break;
    
    int lenEnd = lcsrInfo->lenStart[i + 1];
    int enter = length % M;
    const int iterations = (enter > 0) + (length / M);
    
    for (int r = lcsrInfo->lenStart[i]; r < lenEnd; r++) {
      int rowIndex = rows[r];
      double sum = 0.0;
      int n = iterations;
      
      vals += enter;
      cols += enter;

      switch (enter) {
        case 0: do {
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
        } while (--n > 0);
      }
      w[rowIndex] += sum;
    }
  }
}

void DuffsDeviceLCSR32::spmv(double* __restrict v, double* __restrict w) {
  const int M = 32;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
  
  // TODO: Fix this for multi-threading
  int k = 0;
  for (int i = 0; i < lcsrInfo->numLengths; i++) {
    int length = lcsrInfo->length[i];
    if (length == 0) break;
    
    int lenEnd = lcsrInfo->lenStart[i + 1];
    int enter = length % M;
    const int iterations = (enter > 0) + (length / M);
    
    for (int r = lcsrInfo->lenStart[i]; r < lenEnd; r++) {
      int rowIndex = rows[r];
      double sum = 0.0;
      int n = iterations;
      
      vals += enter;
      cols += enter;
      
      switch (enter) {
        case 0: do {
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
        } while (--n > 0);
      }
      w[rowIndex] += sum;
    }
  }
}
