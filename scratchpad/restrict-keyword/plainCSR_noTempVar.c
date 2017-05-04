void spMV_CSR1(double *vals, int *cols, int *rows, int N, double *v, double *w) {
  for (int i = 0; i < N; i++) {
    for (int k = rows[i]; k < rows[i+1]; k++) {
      w[i] += vals[k] * v[cols[k]];
    }
  }
}
