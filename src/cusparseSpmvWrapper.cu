#include "cusparseSpmvWrapper.hu"

using namespace thundercat;


CusparseSpmvWrapper::~CusparseSpmvWrapper() {
  cudaFree(rowIndexDevPtr);
  cudaFree(colIndexDevPtr);
  cudaFree(valDevPtr);

  cudaFree(x);
  cudaFree(y);

  cusparseDestroyMatDescr(descr);
  cusparseDestroy(handle);

}

void CusparseSpmvWrapper::init() {
  handle = 0;
  descr = 0;

  cusparseCreate(&handle);

  cusparseStatus_t status = cusparseCreateMatDescr(&descr);
  cusparseSetMatType(descr, CUSPARSE_MATRIX_TYPE_GENERAL);
  cusparseSetMatIndexBase(descr, CUSPARSE_INDEX_BASE_ZERO);
}

void CusparseSpmvWrapper::preprocess(int nnz, int m, int n, int * rowPtr, int* colIdx, double* values) {

  M = m;
  N = n;
  NNZ = nnz;

  cudaMalloc((void**)&rowIndexDevPtr, (N + 1) * sizeof(int));
  cudaMalloc((void**)&colIndexDevPtr, NNZ * sizeof(int));
  cudaMalloc((void**)&valDevPtr, NNZ * sizeof(double));

  cudaMemcpy(rowIndexDevPtr, rowPtr,(size_t)((N + 1)*sizeof(rowPtr[0])),cudaMemcpyHostToDevice);
  cudaMemcpy(colIndexDevPtr, colIdx,(size_t)(NNZ*sizeof(colIdx[0])),cudaMemcpyHostToDevice);
  cudaMemcpy(valDevPtr, values,(size_t)(NNZ*sizeof(values[0])),cudaMemcpyHostToDevice);

  cudaMalloc((void**)&x, M * sizeof(double));
  cudaMalloc((void**)&y, N * sizeof(double));


}

void CusparseSpmvWrapper::spmv(double * v, double * w) {
  double alpha = 1.0;
  double beta = 1.0;

  cudaMemcpy(x, v,(size_t)(M*sizeof(v[0])),cudaMemcpyHostToDevice);

  cusparseDcsrmv(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, M, N, NNZ, &alpha,
                 descr, valDevPtr, rowIndexDevPtr, colIndexDevPtr, x, &beta, y);

  cudaMemcpy(y, w,(size_t)(N*sizeof(y[0])),cudaMemcpyDeviceToHost);

}