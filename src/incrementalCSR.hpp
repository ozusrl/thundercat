#pragma once

#include "method.h"
#include "spmvRegistry.h"
#include <iostream>

using namespace thundercat;
using namespace std;

///
/// Incremental rows array with compression
///
class IncrementalCsr: public CsrSpmvMethod {
public:
  IncrementalCsr();
  
  virtual void spmv(double* __restrict v, double* __restrict w) final;

protected:
  virtual void convertMatrix() final;
  
private:
  template <typename T>
  void spmvICSR(double* __restrict v, double* __restrict w, T* __restrict rows);
protected:
  int sizeOfRowLength;

private:
  std::unique_ptr<CSRMatrix<VALUE_TYPE>> matrix;
};



template <typename T>
void IncrementalCsr::spmvICSR(double* __restrict v, double* __restrict w, T* __restrict rows) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    rows += stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->colIndices + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->values + stripeInfos->at(t).valIndexBegin;
  
    int k = stripeInfos->at(t).valIndexBegin;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const T length = rows[i];
      for (T j = 0; j < length; j++) {
        sum += matrix->values[k] * v[matrix->colIndices[k]];
        k++;
      }
      w[i] += sum;
    }
  }
}

