void spMV_CSR1(double* __restrict vals, int* __restrict cols, int* __restrict rows,
               int N, double* __restrict v, double* __restrict w) {
  for (int i = 0; i < N; i++) {
    double ww = 0.0;
    int k;
    for (k = rows[i]; k < rows[i + 1] - 3; k += 4) {
      ww += vals[k] * v[cols[k]];
      ww += vals[k+1] * v[cols[k+1]];
      ww += vals[k+2] * v[cols[k+2]];
      ww += vals[k+3] * v[cols[k+3]];
    }
    for (; k < rows[i + 1]; k++) {
      ww += vals[k] * v[cols[k]];
    }
    w[i] += ww;
  }
}
