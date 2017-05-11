void spMV_CSR1(double *vals, int *cols, int *rows, int N, double *v, double *w) {
  for (int i = 0; i < N; i++) {
    double ww1 = 0.0;
    double ww2 = 0.0;
    double ww3 = 0.0;
    double ww4 = 0.0;
    int k;
    for (k = rows[i]; k < rows[i + 1] - 3; k += 4) {
      ww1 += vals[k] * v[cols[k]];
      ww2 += vals[k+1] * v[cols[k+1]];
      ww3 += vals[k+2] * v[cols[k+2]];
      ww4 += vals[k+3] * v[cols[k+3]];
    }
    double ww = ww1 + ww2 + ww3 + ww4;
    for (; k < rows[i + 1]; k++) {
      ww += vals[k] * v[cols[k]];
    }
    w[i] += ww;
  }
}
