void spMV_CSR1(double* __restrict vals, int* __restrict cols, int* __restrict rows,
               int N, double* __restrict v, double* __restrict w) {
  for (int i = 0; i < N; i++) {
    for (int k = rows[i]; k < rows[i+1]; k++) {
      w[i] += vals[k] * v[cols[k]];
    }
  }
}
