extern int matrixRows[];
extern int matrixCols[];
extern double matrixVals[];

void multByM(double v[], double w[])  {
  omp_set_num_threads(12);
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
  #pragma omp section
  {
    multByM4(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM5(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM6(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM7(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM8(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM9(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM10(v, w, matrixRows, matrixCols, matrixVals);
  }
  #pragma omp section
  {
    multByM11(v, w, matrixRows, matrixCols, matrixVals);
  }
  }
}
