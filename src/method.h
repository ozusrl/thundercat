#ifndef _METHOD_H_
#define _METHOD_H_

#include "mmmatrix.hpp"
#include <unordered_map>
#include <iostream>
#include <map>
#include "asmjit/asmjit.h"
#ifdef OPENMP_EXISTS
#include "omp.h"
#endif

#define VALUE_TYPE double


namespace thundercat {
    
  
  class SpmvMethod {
  public:
    virtual void init(unsigned int numThreads) = 0;
  
    virtual ~SpmvMethod() {}
  
    virtual void preprocess(MMMatrix<VALUE_TYPE> &matrix) = 0;
  
    virtual void spmv(double *__restrict v, double *__restrict w) = 0;
  };
  
  class CsrSpmvMethod : public SpmvMethod {
  public:
    virtual void init(unsigned int numThreads);
  
    virtual ~CsrSpmvMethod();
  
    virtual void preprocess(MMMatrix<VALUE_TYPE> &matrix);
  
    virtual void spmv(double *__restrict v, double *__restrict w) = 0;
  
  
  protected:
    virtual void analyzeMatrix();
  
    virtual void convertMatrix();
  
    std::vector<MatrixStripeInfo> *stripeInfos;
    unsigned int numPartitions;
  
    std::unique_ptr<CSRMatrix<VALUE_TYPE>> csrMatrix;
  };
  
  ///
  /// PlainCSR
  ///
  class PlainCSR : public CsrSpmvMethod {
  public:
    virtual void spmv(double *__restrict v, double *__restrict w) final;
  
    static const std::string name;
  };
  
  class PlainCSR4 : public CsrSpmvMethod {
  public:
    virtual void spmv(double *__restrict v, double *__restrict w) final;
  
    static const std::string name;
  };
  
  class PlainCSR8 : public CsrSpmvMethod {
  public:
    virtual void spmv(double *__restrict v, double *__restrict w) final;
  
    static const std::string name;
  };
  
  class PlainCSR16 : public CsrSpmvMethod {
  public:
    virtual void spmv(double *__restrict v, double *__restrict w) final;
  
    static const std::string name;
  };
  
  class PlainCSR32 : public CsrSpmvMethod {
  public:
    virtual void spmv(double *__restrict v, double *__restrict w) final;
  
    static const std::string name;
  };
}


#endif
