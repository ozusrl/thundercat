
#include <string>
#include "incrementalCSR.hpp"

REGISTER_METHOD(IncrementalCsr, "icsr")

IncrementalCsr::IncrementalCsr() {
}

void IncrementalCsr::convertMatrix() {
  int *rows = new int[csrMatrix->N];
  unsigned char *rowPtr = (unsigned char *)rows;
  int *cols = csrMatrix->colIndices;
  double *vals = csrMatrix->values;

  // newly created matrix will take the ownership of colIndices and values, set these two to NULL to avoid double free
  // problem.
  csrMatrix->colIndices = NULL;
  csrMatrix->values = NULL;

  int maxRowLength = 0;
  for (int i = 0; i < csrMatrix->N; i++) {
    int length = csrMatrix->rowPtr[i + 1] - csrMatrix->rowPtr[i];
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
      int length = csrMatrix->rowPtr[i + 1] - csrMatrix->rowPtr[i];
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

  matrix = std::make_unique<CSRMatrix<VALUE_TYPE>>(rows, cols, vals, csrMatrix->N, csrMatrix->M, csrMatrix->NZ);
}

void IncrementalCsr::spmv(double* __restrict v, double* __restrict w) {
  const int *rows = matrix->rowPtr;
  switch(sizeOfRowLength) {
    case 1:
      spmvICSR(v, w, (unsigned char *)rows); break;
    case 2:
      spmvICSR(v, w, (unsigned short *)rows); break;
    default:
      spmvICSR(v, w, (unsigned int *)rows); break;
  }
}