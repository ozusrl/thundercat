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
