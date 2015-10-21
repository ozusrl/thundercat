extern int matrixRows[];
extern int matrixCols[];
extern double matrixVals[];

void multByM(double v[], double w[])  {
  omp_set_num_threads(4);
#pragma omp parallel sections shared(v,w)
  {
  #pragma omp section
  {
    multByM0(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM1(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM2(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM3(v, w, matrixRows, matrixCols, matrixVals);
  }
  }
}
