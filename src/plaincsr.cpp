#include "method.h"
#include "spmvRegistry.h"

using namespace thundercat;
using namespace std;


void PlainCSR::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      for (int k = csrMatrix->rowPtr[i]; k < csrMatrix->rowPtr[i + 1]; k++) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
      }
      w[i] += ww;
    }
  }
}

REGISTER_METHOD(PlainCSR, "pcsr")


void PlainCSR4::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = csrMatrix->rowPtr[i]; k < csrMatrix->rowPtr[i + 1] - 3; k += 4) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
        ww += csrMatrix->values[k+1] * v[csrMatrix->colIndices[k+1]];
        ww += csrMatrix->values[k+2] * v[csrMatrix->colIndices[k+2]];
        ww += csrMatrix->values[k+3] * v[csrMatrix->colIndices[k+3]];
      }
      for (; k < csrMatrix->rowPtr[i + 1]; k++) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
      }
      w[i] += ww;
    }
  }
}
REGISTER_METHOD(PlainCSR4, "pcsr4")


void PlainCSR8::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = csrMatrix->rowPtr[i]; k < csrMatrix->rowPtr[i + 1] - 7; k += 8) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
        ww += csrMatrix->values[k+1] * v[csrMatrix->colIndices[k+1]];
        ww += csrMatrix->values[k+2] * v[csrMatrix->colIndices[k+2]];
        ww += csrMatrix->values[k+3] * v[csrMatrix->colIndices[k+3]];
        ww += csrMatrix->values[k+4] * v[csrMatrix->colIndices[k+4]];
        ww += csrMatrix->values[k+5] * v[csrMatrix->colIndices[k+5]];
        ww += csrMatrix->values[k+6] * v[csrMatrix->colIndices[k+6]];
        ww += csrMatrix->values[k+7] * v[csrMatrix->colIndices[k+7]];
      }
      for (; k < csrMatrix->rowPtr[i + 1]; k++) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
      }
      w[i] += ww;
    }
  }
}
REGISTER_METHOD(PlainCSR8, "pcsr8")


void PlainCSR16::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = csrMatrix->rowPtr[i]; k < csrMatrix->rowPtr[i + 1] - 15; k += 16) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
        ww += csrMatrix->values[k+1] * v[csrMatrix->colIndices[k+1]];
        ww += csrMatrix->values[k+2] * v[csrMatrix->colIndices[k+2]];
        ww += csrMatrix->values[k+3] * v[csrMatrix->colIndices[k+3]];
        ww += csrMatrix->values[k+4] * v[csrMatrix->colIndices[k+4]];
        ww += csrMatrix->values[k+5] * v[csrMatrix->colIndices[k+5]];
        ww += csrMatrix->values[k+6] * v[csrMatrix->colIndices[k+6]];
        ww += csrMatrix->values[k+7] * v[csrMatrix->colIndices[k+7]];
        ww += csrMatrix->values[k+8] * v[csrMatrix->colIndices[k+8]];
        ww += csrMatrix->values[k+9] * v[csrMatrix->colIndices[k+9]];
        ww += csrMatrix->values[k+10] * v[csrMatrix->colIndices[k+10]];
        ww += csrMatrix->values[k+11] * v[csrMatrix->colIndices[k+11]];
        ww += csrMatrix->values[k+12] * v[csrMatrix->colIndices[k+12]];
        ww += csrMatrix->values[k+13] * v[csrMatrix->colIndices[k+13]];
        ww += csrMatrix->values[k+14] * v[csrMatrix->colIndices[k+14]];
        ww += csrMatrix->values[k+15] * v[csrMatrix->colIndices[k+15]];
      }
      for (; k < csrMatrix->rowPtr[i + 1]; k++) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
      }
      w[i] += ww;
    }
  }
}
REGISTER_METHOD(PlainCSR16, "pcsr16")


void PlainCSR32::spmv(double* __restrict v, double* __restrict w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      int k;
      for (k = csrMatrix->rowPtr[i]; k < csrMatrix->rowPtr[i + 1] - 31; k += 32) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
        ww += csrMatrix->values[k+1] * v[csrMatrix->colIndices[k+1]];
        ww += csrMatrix->values[k+2] * v[csrMatrix->colIndices[k+2]];
        ww += csrMatrix->values[k+3] * v[csrMatrix->colIndices[k+3]];
        ww += csrMatrix->values[k+4] * v[csrMatrix->colIndices[k+4]];
        ww += csrMatrix->values[k+5] * v[csrMatrix->colIndices[k+5]];
        ww += csrMatrix->values[k+6] * v[csrMatrix->colIndices[k+6]];
        ww += csrMatrix->values[k+7] * v[csrMatrix->colIndices[k+7]];
        ww += csrMatrix->values[k+8] * v[csrMatrix->colIndices[k+8]];
        ww += csrMatrix->values[k+9] * v[csrMatrix->colIndices[k+9]];
        ww += csrMatrix->values[k+10] * v[csrMatrix->colIndices[k+10]];
        ww += csrMatrix->values[k+11] * v[csrMatrix->colIndices[k+11]];
        ww += csrMatrix->values[k+12] * v[csrMatrix->colIndices[k+12]];
        ww += csrMatrix->values[k+13] * v[csrMatrix->colIndices[k+13]];
        ww += csrMatrix->values[k+14] * v[csrMatrix->colIndices[k+14]];
        ww += csrMatrix->values[k+15] * v[csrMatrix->colIndices[k+15]];
        ww += csrMatrix->values[k+16] * v[csrMatrix->colIndices[k+16]];
        ww += csrMatrix->values[k+17] * v[csrMatrix->colIndices[k+17]];
        ww += csrMatrix->values[k+18] * v[csrMatrix->colIndices[k+18]];
        ww += csrMatrix->values[k+19] * v[csrMatrix->colIndices[k+19]];
        ww += csrMatrix->values[k+20] * v[csrMatrix->colIndices[k+20]];
        ww += csrMatrix->values[k+21] * v[csrMatrix->colIndices[k+21]];
        ww += csrMatrix->values[k+22] * v[csrMatrix->colIndices[k+22]];
        ww += csrMatrix->values[k+23] * v[csrMatrix->colIndices[k+23]];
        ww += csrMatrix->values[k+24] * v[csrMatrix->colIndices[k+24]];
        ww += csrMatrix->values[k+25] * v[csrMatrix->colIndices[k+25]];
        ww += csrMatrix->values[k+26] * v[csrMatrix->colIndices[k+26]];
        ww += csrMatrix->values[k+27] * v[csrMatrix->colIndices[k+27]];
        ww += csrMatrix->values[k+28] * v[csrMatrix->colIndices[k+28]];
        ww += csrMatrix->values[k+29] * v[csrMatrix->colIndices[k+29]];
        ww += csrMatrix->values[k+30] * v[csrMatrix->colIndices[k+30]];
        ww += csrMatrix->values[k+31] * v[csrMatrix->colIndices[k+31]];
      }
      for (; k < csrMatrix->rowPtr[i + 1]; k++) {
        ww += csrMatrix->values[k] * v[csrMatrix->colIndices[k]];
      }
      w[i] += ww;
    }
  }
}
REGISTER_METHOD(PlainCSR32, "pcsr32")