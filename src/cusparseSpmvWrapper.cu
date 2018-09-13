#include "cusparseSpmvWrapper.hu"

using namespace thundercat;

CusparseSpmvWrapper* thundercat::newCusparseSpmvWrapper() {
  return new CusparseSpmvWrapper();
};

void thundercat::deleteCusparseSpmvWrapper(CusparseSpmvWrapper* wrapper) {
  delete wrapper;
}

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

  cudaError_t error = cudaMalloc((void**)&rowIndexDevPtr, (N + 1) * sizeof(int));
  error = cudaMalloc((void**)&colIndexDevPtr, NNZ * sizeof(int));
  error = cudaMalloc((void**)&valDevPtr, NNZ * sizeof(double));

  error = cudaMemcpy((void *)rowIndexDevPtr, (void*)rowPtr, (size_t) ((N + 1) * sizeof(int)), cudaMemcpyHostToDevice);
  error = cudaMemcpy((void*)colIndexDevPtr, (void*)colIdx, (size_t) (NNZ * sizeof(int)), cudaMemcpyHostToDevice);
  error = cudaMemcpy((void*) valDevPtr, (void*)values, (size_t) (NNZ * sizeof(double)), cudaMemcpyHostToDevice);

  error = cudaMalloc((void**)&x, M * sizeof(double));
  error = cudaMalloc((void**)&y, N * sizeof(double));


}

void CusparseSpmvWrapper::spmv(double * v, double * w) {
  double alpha = 1.0;
  double beta = 1.0;

  cudaMemcpy((void*) x, (void*) v,(size_t)(M*sizeof(double)),cudaMemcpyHostToDevice);

  cusparseDcsrmv(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, M, N, NNZ, &alpha,
                 descr, valDevPtr, rowIndexDevPtr, colIndexDevPtr, x, &beta, y);

  cudaMemcpy((void*) w, (void*) y,(size_t)(N*sizeof(double)),cudaMemcpyDeviceToHost);

}