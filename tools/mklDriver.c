extern int matrixRows[];
extern int matrixCols[];
extern double matrixVals[];
extern int n;

#include <mkl.h>

void multByM(double v[], double w[])  {
  mkl_set_num_threads_local(NUMTHREADS); // This value is replaced by a sed command.

  double alpha = 1.0;
  double beta = 1.0;
  int *ptrb = matrixRows;
  int *ptre = ptrb+1;
  char trans[] = "N";
  char matdescra[] = "G__C";
  mkl_dcsrmv(trans, &n, &n, &alpha, matdescra, matrixVals, matrixCols, ptrb, ptre, v, &beta, w);
}
