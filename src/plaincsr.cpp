#include "method.h"
#ifdef OMP_EXISTS
#include "omp.h"
#endif

using namespace thundercat;
using namespace std;

void PlainCSR::spmv(double *v, double *w) {
#pragma omp parallel for
  for (unsigned int t = 0; t < stripeInfos->size(); t++) {
    int rowIndexBegin = stripeInfos->at(t).rowIndexBegin;
    int rowIndexEnd = stripeInfos->at(t).rowIndexEnd;
    for (int i = rowIndexBegin; i < rowIndexEnd; i++) {
      double ww = 0.0;
      for (int k = matrix->rows[i]; k < matrix->rows[i + 1]; k++) {
        ww += matrix->vals[k] * v[matrix->cols[k]];
      }
      w[i] += ww;
    }
  }
}
