#include <cusp/csr_matrix.h>
#include <cusp/multiply.h>
#include "cuspAdapter.hu"

using namespace thundercat;
void CuspAdapter::preprocess( int m, int n, int nnz, int * rowPtr, int * colIndx, double * values) {
  M = m;
  N = n;
  NNZ = nnz;

  cudaMalloc(&devRowPtr, M * sizeof(int));
  cudaMalloc(&devColIndx, NNZ * sizeof(int));
  cudaMalloc(&devValues, NNZ * sizeof(double));
  cudaMalloc(&devX, M * sizeof(double));
  cudaMalloc(&devY, N * sizeof(double));

  cudaMemcpy(devRowPtr,  rowPtr, M * sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(devColIndx, colIndx, NNZ * sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(devValues, values, NNZ * sizeof(double), cudaMemcpyHostToDevice);

}

void CuspAdapter::spmv(double * v, double * w) {
  cudaMemcpy(devX, v, M * sizeof(double), cudaMemcpyHostToDevice);

    // *NOTE* raw pointers must be wrapped with thrust::device_ptr!
  thrust::device_ptr<int>   wrapped_device_Ap(devRowPtr);
  thrust::device_ptr<int>   wrapped_device_Aj(devColIndx);
  thrust::device_ptr<double> wrapped_device_Ax(devValues);
  thrust::device_ptr<double> wrapped_device_x(devX);
  thrust::device_ptr<double> wrapped_device_y(devY);

  // use array1d_view to wrap the individual arrays
  typedef typename cusp::array1d_view< thrust::device_ptr<int>   > DeviceIndexArrayView;
  typedef typename cusp::array1d_view< thrust::device_ptr<double> > DeviceValueArrayView;

  DeviceIndexArrayView row_offsets   (wrapped_device_Ap, wrapped_device_Ap + M);
  DeviceIndexArrayView column_indices(wrapped_device_Aj, wrapped_device_Aj + NNZ);
  DeviceValueArrayView values        (wrapped_device_Ax, wrapped_device_Ax + NNZ);
  DeviceValueArrayView x (wrapped_device_x, wrapped_device_x + M);
  DeviceValueArrayView y (wrapped_device_y, wrapped_device_y + N);


  typedef cusp::csr_matrix_view<DeviceIndexArrayView,
      DeviceIndexArrayView,
      DeviceValueArrayView> DeviceView;
  DeviceView A(M, N, NNZ, row_offsets, column_indices, values);


  cusp::multiply(A, x, y);
  cudaMemcpy(w, devY, N * sizeof(double), cudaMemcpyDeviceToHost);
}

CuspAdapter* thundercat::newCuspAdapter() {
  return new CuspAdapter();
}

void thundercat::deleteCuspAdapter(CuspAdapter* handle) {
  delete handle;
}
