void spmv(double* __restrict vals, int* __restrict cols, int* __restrict rows,
          int N, double* __restrict v, double* __restrict w) {
  const int M = 4;
  for (int i = 0; i < N; i++) {
    double sum = 0.0;
    int k = rows[i];
    const int length = rows[i + 1] - rows[i];
    int n =  length / M;
    const int currentCase = length % M;
    switch (currentCase) {
      do {
        sum += vals[k] * v[cols[k]]; k++;
      case 3:
        sum += vals[k] * v[cols[k]]; k++;
      case 2:
        sum += vals[k] * v[cols[k]]; k++;
      case 1:
        sum += vals[k] * v[cols[k]]; k++;
      case 0:
        ;
      }
      while (--n >= 0);
    }
    w[i] += sum;
  }
}
