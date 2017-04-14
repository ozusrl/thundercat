#include "matrix.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace thundercat;
using namespace std;

Matrix::Matrix(int *rows, int *cols, double *vals, unsigned long n, unsigned long nz):
  rows(rows), cols(cols), vals(vals), n(n), nz(nz) { 
  numRows = n;
  numCols = nz;
  numVals = nz;
}

Matrix::~Matrix() {
  delete[] rows;
  delete[] cols;
  delete[] vals;
}

// Caller of this method is responsible for destructing the
// returned matrix.
Matrix* Matrix::readMatrixFromFile(string fileName) {
  ifstream mmFile(fileName.c_str());
  if (!mmFile.is_open()) {
    std::cerr << "Problem with file " << fileName << ".\n";
    exit(1);
  }
  string headerLine;
  // consume the comments until we reach the size info
  while (mmFile.good()) {
    getline (mmFile, headerLine);
    if (headerLine[0] != '%') break;
  }
  
  // Read N, M, NZ
  stringstream header(headerLine, ios_base::in);
  int n, m, nz;
  header >> n >> m >> nz;
  if (n != m) {
    std::cerr << "Only square matrices are accepted.\n";
    exit(1);
  }
  
  // Read rows, cols, vals
  MMMatrix matrix(n);
  int row; int col; double val;
  
  string line;
  for (int i = 0; i < nz; ++i) {
    getline(mmFile, line);
    stringstream linestream(line, ios_base::in);
    linestream >> row >> col;
    // Pattern (i.e. connectivity) matrices do not contain val entry.
    // Such matrices are filled in with 1.0
    linestream >> val;
    if (linestream.fail())
      val = 1.0;
    // adjust to zero index
    matrix.add(row-1, col-1, val);
  }
  mmFile.close();
  matrix.normalize();
  Matrix *csrMatrix = matrix.toCSRMatrix();
  return csrMatrix;
}


vector<MatrixStripeInfo> *Matrix::getStripeInfos(unsigned int numPartitions) {
  if (stripeInfos.size() != 0) {
    cout << "I was not expecting getStripeInfos to be called multiple times.\n";
  }
  // Split the matrix
  unsigned long chunkSize = this->numVals / numPartitions;
  unsigned int rowIndex = 0;
  unsigned long valIndex = 0;
  for (int partitionIndex = 0; partitionIndex < numPartitions; ++partitionIndex) {
    unsigned int rowIndexStart = rowIndex;
    unsigned long valIndexStart = valIndex;
    unsigned long numElementsCovered = 0;
    while(numElementsCovered < chunkSize && rowIndex < this->n) {
      numElementsCovered += this->rows[rowIndex+1] - this->rows[rowIndex];
      rowIndex++;
    }
    valIndex += numElementsCovered;
    if (partitionIndex == numPartitions - 1) {
      rowIndex = this->n;
      valIndex = this->nz;
    }
    
    MatrixStripeInfo stripeInfo;
    stripeInfo.rowIndexBegin = rowIndexStart;
    stripeInfo.rowIndexEnd = rowIndex;
    stripeInfo.valIndexBegin = valIndexStart;
    stripeInfo.valIndexEnd = valIndex;
    stripeInfos.push_back(stripeInfo);
  }
  return &stripeInfos;
}

void Matrix::print() {
  cout << "int numMatrixRows = " << n << ";\n";
  cout << "int numMatrixValues = " << nz << ";\n";
  if (numCols == 0) {
    cout << "int *matrixCols = 0;\n";
  } else {
    cout << "int matrixCols[" << numCols << "] = {\n";
    for(int i = 0; i < numCols; ++i) {
      cout << cols[i] << ", \n";
    }
    cout << "};\n";
  }
  if (numRows == 0) {
    cout << "int *matrixRows = 0;\n";
  } else {
    cout << "int matrixRows[" << numRows << "] = {\n";
    for(int i = 0; i < numRows; ++i) {
      cout << rows[i] << ", \n";
    }
    cout << "};\n";
  }
  if (numVals == 0) {    
    cout << "double *matrixVals = 0;\n";
  } else {
    cout << "double matrixVals[" << numVals << "] = {\n";
    for(int i = 0; i < numVals; ++i) {
      if(vals[i] == 0) 
        cout << "0.0, \n";
      else 
        cout << vals[i] << ", \n";
    }
    cout << "};\n";      
  }
}  

bool MMElement::compare(const MMElement &elt1, const MMElement &elt2) {
  if (elt1.row < elt2.row) return true;
  else if (elt2.row < elt1.row) return false;
  else return elt1.col < elt2.col;
}

bool MMElement::compareCol(int limit){
  return (col < limit);
}

MMMatrix::MMMatrix(unsigned long n) {
  this->n = n;
}

MMMatrix::~MMMatrix() {

}

void MMMatrix::sort() {
  std::sort(elts.begin(), elts.end(), MMElement::compare);
}

void MMMatrix::add(int row, int col, double val) {
  elts.push_back(MMElement(row, col, val));
}

void MMMatrix::normalize() {
  sort();
}
    
void MMMatrix::print() {
  cout << n << " " << n << " " << elts.size() << "\n";
  for (MMElement &elt : elts) {
    cout << elt.row << " "
         << elt.col << " "
         << elt.val << "\n";
  }
}

void MMMatrix::printMTX() {
  cout << n << " " << n << " " << elts.size() << "\n";
  for (MMElement &elt : elts) {
    cout << (elt.row + 1) << " "
         << (elt.col + 1) << " "
         << elt.val << "\n";
  }
}

Matrix* MMMatrix::toCSRMatrix() {
  long sz = elts.size();
  int *rows = new int[n+1];
  rows[0] = 0;
  int *cols = new int[sz]; 
  double *vals = new double[sz]; 

  unsigned int eltIndex = 0;
  unsigned int rowIndex = 0;
  for (auto &elt : elts) {
    cols[eltIndex] = elt.col;
    vals[eltIndex] = elt.val;
    while (rowIndex != elt.row) {
      rows[rowIndex+1] = eltIndex;
      rowIndex++;
    }
    eltIndex++;
  }
  while (rowIndex < n) {
    rows[rowIndex+1] = eltIndex;
    rowIndex++;
  }
  rows[n] = eltIndex;
  
  Matrix *matrix = new Matrix(rows, cols, vals, n, sz);
  matrix->numRows = n + 1;
  return matrix;
}

