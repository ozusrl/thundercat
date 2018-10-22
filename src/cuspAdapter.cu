#include <cusp/csr_matrix.h>
#include <cusp/multiply.h>
#include "cuspAdapter.hu"

using namespace thundercat;
void CuspAdapter::preprocess( int m, int n, int nnz, int * rowPtr, int * colIndx, double * values) {
  M = m;
  N = n;
  NNZ = nnz;

  int *devRowPtr;
  int *devColIndx;
  double *devValues;

  cudaMalloc(&devRowPtr, (N+1) * sizeof(int));
  cudaMalloc(&devColIndx, NNZ * sizeof(int));
  cudaMalloc(&devValues, NNZ * sizeof(double));
  cudaMalloc(&devX, M * sizeof(double));
  cudaMalloc(&devY, N * sizeof(double));

  cudaMemcpy(devRowPtr,  rowPtr, (N+1) * sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(devColIndx, colIndx, NNZ * sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(devValues, values, NNZ * sizeof(double), cudaMemcpyHostToDevice);

  // *NOTE* raw pointers must be wrapped with thrust::device_ptr!
  thrust::device_ptr<int>   wrapped_device_Ap(devRowPtr);
  thrust::device_ptr<int>   wrapped_device_Aj(devColIndx);
  thrust::device_ptr<double> wrapped_device_Ax(devValues);
  thrust::device_ptr<double> wrapped_device_x(devX);
  thrust::device_ptr<double> wrapped_device_y(devY);



  DeviceIndexArrayView row_offsets(wrapped_device_Ap, wrapped_device_Ap + N + 1);
  DeviceIndexArrayView column_indices(wrapped_device_Aj, wrapped_device_Aj + NNZ);
  DeviceValueArrayView values_array        (wrapped_device_Ax, wrapped_device_Ax + NNZ);
  DeviceValueArrayView x_local(wrapped_device_x, wrapped_device_x + M);
  DeviceValueArrayView y_local(wrapped_device_y, wrapped_device_y + N);

  DeviceView A_local(M, N, NNZ, row_offsets, column_indices, values_array);
  A = A_local;
  x = x_local;
  y = y_local;

}

void CuspAdapter::setX(double * v) {
  cudaMemcpy(devX, v, M * sizeof(double), cudaMemcpyHostToDevice);
  cudaThreadSynchronize();
}

void CuspAdapter::getY(double * w) {
  cudaMemcpy(w, devY, N * sizeof(double), cudaMemcpyDeviceToHost);
  cudaThreadSynchronize();
}

void CuspAdapter::spmv() {
  cusp::multiply(A, x, y);
  cudaThreadSynchronize();
}

CuspAdapter* thundercat::newCuspAdapter() {
  return new CuspAdapter();
}

void thundercat::deleteCuspAdapter(CuspAdapter* handle) {
  delete handle;
}
