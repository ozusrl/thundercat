extern int matrixRows[];
extern int matrixCols[];
extern double matrixVals[];

void multByM(double v[], double w[])  {
    multByM0(v, w, matrixRows, matrixCols, matrixVals);
}
