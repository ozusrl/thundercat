#ifndef _SPMV_MATRIX_H
#define _SPMV_MATRIX_H

#include <cstdlib> // defines NULL
#include <vector>
#include <map>
#include <set>
#include <utility>

namespace thundercat {
  typedef struct {
    unsigned int rowIndexBegin;
    unsigned int rowIndexEnd;
    unsigned long valIndexBegin;
    unsigned long valIndexEnd;
  } MatrixStripeInfo;
  
  class Matrix final {
  public:
    int* __restrict rows;
    int* __restrict cols;
    double* __restrict vals;
    unsigned long n;
    unsigned long nz;
    // For some representations, numRows, numCols, numVals
    // may not be the same as n, nz.
    unsigned long numRows, numCols, numVals;
    
    Matrix(int* __restrict rows, int* __restrict cols, double* __restrict vals,
           unsigned long n, unsigned long nz);

    ~Matrix();
    
    std::vector<MatrixStripeInfo> *getStripeInfos(unsigned int numPartitions);
    
    void print();
      
    static Matrix* readMatrixFromFile(std::string fileName);

  private:
    std::vector<MatrixStripeInfo> stripeInfos;
  };

  class MMElement {
  public:
    int row; int col; double val;
    
    MMElement(int row, int col, double val):
      row(row), col(col), val(val) {}
    
    static bool compare(const MMElement &elt1, const MMElement &elt2);
    bool compareCol(int col);
  };

  class MMMatrix final {
  private:
    std::vector<MMElement> elts;
    unsigned long n;

  public:
    MMMatrix(unsigned long n);

    ~MMMatrix();

    void add(int row, int col, double val);
    void normalize();
    void print();
    void printMTX();
    
    // Return a matrix in the CSR format
    Matrix* toCSRMatrix();

  private:
    void sort();
  };
  
  class LCSRInfo final {
  public:
    int numLengths;
    int *length;
    int *lenStart;
    
    LCSRInfo(int numLengths, int *length, int *lenStart);
    
    ~LCSRInfo();
  };
  


}

#endif
