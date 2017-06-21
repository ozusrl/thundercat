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
  
  matrix = new Matrix(rows, cols, vals, csrMatrix->n, csrMatrix->m, csrMatrix->nz);
  lcsrInfo = new LCSRInfo(numLengths, length, lenStart);
}

void DuffsDeviceLCSR4::spmv(double* __restrict v, double* __restrict w) {
  const int M = 4;
  const int *rows = matrix->rows;
  const int *cols = matrix->cols;
  const double *vals = matrix->vals;
  
  // TODO: Fix this for multi-threading
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
          sum += vals[-4] * v[cols[-4]];
        case 3:
          sum += vals[-3] * v[cols[-3]];
        case 2:
          sum += vals[-2] * v[cols[-2]];
        case 1:
          sum += vals[-1] * v[cols[-1]];
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
        } while (--n > 0);
      }
      w[rowIndex] += sum;
    }
  }
}
