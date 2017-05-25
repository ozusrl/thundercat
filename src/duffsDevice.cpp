#include "method.h"

using namespace thundercat;
using namespace std;

#define branch(i) case i: sum += vals[-i] * v[cols[-i]];

void DuffsDevice4::spmv(double* __restrict v, double* __restrict w) {
  const int M = 4;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    
    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      const int currentCase = length % M;
      
      vals += currentCase;
      cols += currentCase;
      
      switch (currentCase) {
          do {
            vals += 4; cols += 4;
            sum += vals[-4] * v[cols[-4]];
            branch(3)
            branch(2)
            branch(1)
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDevice8::spmv(double* __restrict v, double* __restrict w) {
  const int M = 8;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;

    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      const int currentCase = length % M;

      vals += currentCase;
      cols += currentCase;
      
      switch (currentCase) {
          do {
            vals += 8; cols += 8;
            sum += vals[-8] * v[cols[-8]];
            branch(7)
            branch(6)
            branch(5)
            branch(4)
            branch(3)
            branch(2)
            branch(1)
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDevice16::spmv(double* __restrict v, double* __restrict w) {
  const int M = 16;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;

    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      const int currentCase = length % M;

      vals += currentCase;
      cols += currentCase;
      
      switch (currentCase) {
          do {
            vals += 16; cols += 16;
            sum += vals[-16] * v[cols[-16]];
            branch(15)
            branch(14)
            branch(13)
            branch(12)
            branch(11)
            branch(10)
            branch(9)
            branch(8)
            branch(7)
            branch(6)
            branch(5)
            branch(4)
            branch(3)
            branch(2)
            branch(1)
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}

void DuffsDevice32::spmv(double* __restrict v, double* __restrict w) {
  const int M = 32;
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;

    const int *rows = matrix->rows + stripeInfos->at(t).rowIndexBegin;
    int *cols = matrix->cols + stripeInfos->at(t).valIndexBegin;
    double *vals = matrix->vals + stripeInfos->at(t).valIndexBegin;
    
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double sum = 0.0;
      const int length = rows[i + 1] - rows[i];
      int n =  length / M;
      const int currentCase = length % M;
      
      vals += currentCase;
      cols += currentCase;

      switch (currentCase) {
          do {
            vals += 32; cols += 32;
            sum += vals[-32] * v[cols[-32]];
            branch(31)
            branch(30)
            branch(29)
            branch(28)
            branch(27)
            branch(26)
            branch(25)
            branch(24)
            branch(23)
            branch(22)
            branch(21)
            branch(20)
            branch(19)
            branch(18)
            branch(17)
            branch(16)
            branch(15)
            branch(14)
            branch(13)
            branch(12)
            branch(11)
            branch(10)
            branch(9)
            branch(8)
            branch(7)
            branch(6)
            branch(5)
            branch(4)
            branch(3)
            branch(2)
            branch(1)
          case 0:
            ;
          }
          while (--n >= 0);
      }
      w[i] += sum;
    }
  }
}
