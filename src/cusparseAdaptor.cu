#include "cusparseAdaptor.hu"
#include <iostream>

using namespace thundercat;

CusparseAdaptor* thundercat::newCusparseAdaptor() {
  return new CusparseAdaptor();
};

void thundercat::deleteCusparseAdaptor(CusparseAdaptor* adaptor) {
  delete adaptor;
}

CusparseAdaptor::~CusparseAdaptor() {
  cudaFree(rowIndexDevPtr);
  cudaFree(colIndexDevPtr);
  cudaFree(valDevPtr);

  cudaFree(x);
  cudaFree(y);

  cusparseDestroyMatDescr(descr);
  cusparseDestroy(handle);
}

void CusparseAdaptor::init() {
  handle = 0;
  descr = 0;

  cusparseCreate(&handle);

  cusparseStatus_t status = cusparseCreateMatDescr(&descr);
  cusparseSetMatType(descr, CUSPARSE_MATRIX_TYPE_GENERAL);
  cusparseSetMatIndexBase(descr, CUSPARSE_INDEX_BASE_ZERO);
}

void CusparseAdaptor::preprocess(int nnz, int m, int n, int * rowPtr, int* colIdx, double* values) {

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
  error = cudaMemset(y, 0, N * sizeof(double));
}

void CusparseAdaptor::setX(double *v) {
  cudaMemcpy((void*) x, (void*) v,(size_t)(M*sizeof(double)),cudaMemcpyHostToDevice);
  cudaThreadSynchronize();
}

void CusparseAdaptor::getY(double *w) {
  cudaMemcpy((void*) w, (void*) y,(size_t)(N*sizeof(double)),cudaMemcpyDeviceToHost);
  cudaThreadSynchronize();
}

void CusparseAdaptor::spmv() {
  double alpha = 1.0;
  double beta = 1.0;
  cusparseDcsrmv(handle, CUSPARSE_OPERATION_NON_TRANSPOSE, M, N, NNZ, &alpha,
                 descr, valDevPtr, rowIndexDevPtr, colIndexDevPtr, x, &beta, y);

  cudaThreadSynchronize();
}