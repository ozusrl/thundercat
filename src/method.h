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

  // multByM(v, w, rows, cols, vals)
  typedef void(*MultByMFun)(double*, double*, int*, int*, double*);

  class BaseSpMVMethod {
  public:
      virtual void init(unsigned int numThreads) = 0;

      virtual ~BaseSpMVMethod() {}

      virtual bool isSpecializer() = 0;

      virtual void emitCode() = 0;

//    virtual MATRIX getMethodSpecificMatrix() = 0;

        virtual void processMatrix(std::unique_ptr<MMMatrix<VALUE_TYPE>> matrix) = 0;

        virtual void spmv(double* __restrict v, double* __restrict w) = 0;
  };
  
  class SpMVMethod : public BaseSpMVMethod {
  public:
    virtual void init(unsigned int numThreads);

    virtual ~SpMVMethod();
    
    virtual bool isSpecializer();
    
    virtual void emitCode();
    
//    virtual MATRIX getMethodSpecificMatrix() final;

    virtual void processMatrix(std::unique_ptr<MMMatrix<VALUE_TYPE>> matrix) final;
  
    virtual void spmv(double* __restrict v, double* __restrict w) = 0;

    
  protected:
    virtual void analyzeMatrix();
    virtual void convertMatrix();
    
    std::vector<MatrixStripeInfo> *stripeInfos;
    unsigned int numPartitions;

    std::unique_ptr<CSRMatrix<VALUE_TYPE>> csrMatrix;
    };
  
  ///
  /// MKL
  ///
  class MKL: public SpMVMethod {
  public:
    virtual void init(unsigned int numThreads) final;
    
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };
  
  ///
  /// PlainCSR
  ///
  class PlainCSR: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
    static const std::string name;
  };

  class PlainCSR4: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
    static const std::string name;
  };

  class PlainCSR8: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
    static const std::string name;
  };

  class PlainCSR16: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
    static const std::string name;
  };

  class PlainCSR32: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
    static const std::string name;
  };

  ///
  /// Duff's Device
  ///
  class DuffsDevice4: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };
  
  class DuffsDevice8: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };

  class DuffsDevice16: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };

  class DuffsDevice32: public SpMVMethod {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };

  ///
  /// LCSR utility
  ///
  class RowByNZ {
  public:
    std::vector<int> *getRowIndices();
    void addRowIndex(int index);
    
  private:
    std::vector<int> rowIndices;
  };
  
  // To keep the map keys in ascending order, using the "greater" comparator.
  typedef std::map<unsigned long, RowByNZ, std::greater<unsigned long> > NZtoRowMap;
  
  class LCSRAnalyzer {
  public:
      virtual void analyzeMatrix(CSRMatrix<VALUE_TYPE>& csrMatrix,
                               std::vector<MatrixStripeInfo> stripeInfos,
                               std::vector<NZtoRowMap> &rowByNZLists) final;
  };
  
  ///
  /// Duff's Device for the LCSR format
  ///
  class DuffsDeviceLCSR: public SpMVMethod {
  protected:
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;
    
  protected:
    std::vector<NZtoRowMap> rowByNZLists;
//    LCSRInfo *lcsrInfo;
  };

  class DuffsDeviceLCSR4: public DuffsDeviceLCSR {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };

  class DuffsDeviceLCSR8: public DuffsDeviceLCSR {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };
  
  class DuffsDeviceLCSR16: public DuffsDeviceLCSR {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };

  class DuffsDeviceLCSR32: public DuffsDeviceLCSR {
  public:
    virtual void spmv(double* __restrict v, double* __restrict w) final;
  };

  ///
  /// Specializer
  ///
  class Specializer : public SpMVMethod {
  public:
    virtual void init(unsigned int numThreads) final;
    
    virtual bool isSpecializer() final;
    
    virtual void emitCode() final;
  
    virtual std::vector<asmjit::CodeHolder*> *getCodeHolders() final;

    virtual void spmv(double* __restrict v, double* __restrict w) final;
    
  protected:
    virtual void emitMultByMFunction(unsigned int index) = 0;
    
    std::vector<asmjit::CodeHolder*> codeHolders;
    
    std::vector<MultByMFun> functions;
    
  private:
    void emitConstData();
    
    asmjit::JitRuntime rt;
  };

  ///
  /// CSRbyNZ
  ///
  class CSRbyNZ: public Specializer {
  protected:
    virtual void emitMultByMFunction(unsigned int index);
    virtual void analyzeMatrix() final;
    virtual void convertMatrix();

  protected:
    std::vector<NZtoRowMap> rowByNZLists;
  };
  
  ///
  /// UnrollingWithGOTO
  ///
  class UnrollingWithGOTO: public CSRbyNZ {
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void convertMatrix() final;
  };
  
  ///
  /// Gen OSKI
  ///
  typedef std::map<unsigned long, std::pair<std::vector<std::pair<int, int> >, std::vector<double> > > GroupByBlockPatternMap;
  
  class GenOSKI: public Specializer {
  public:
    GenOSKI(unsigned int b_r, unsigned int b_c);
    
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;

  private:
    unsigned int b_r, b_c;
    std::vector<GroupByBlockPatternMap> groupByBlockPatternMaps;
    std::vector<unsigned int> numBlocks;
  };

  ///
  /// RowPattern
  ///
  // Map row patterns to row indices
  typedef std::map<std::vector<int>, std::vector<int> > RowPatternInfo;
  
  class RowPattern: public Specializer {
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;
    
  private:
    std::vector<RowPatternInfo> patternInfos;
  };
  

  ///
  /// Unfolding
  ///
  class Unfolding: public Specializer {
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;

  private:
    std::vector<std::map<double, unsigned long> > valToIndexMaps;
    std::vector<std::vector<double> > distinctValueLists;
    
    bool hasFewDistinctValues();
  };

  ///
  /// CSRWithGOTO
  ///
  class CSRWithGOTO: public Specializer {
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;
    
  private:
    std::vector<unsigned long> maxRowLengths;
  };

  ///
  /// CSRLenWithGOTO
  ///
  class CSRLenWithGOTO: public Specializer {
  protected:
    virtual void emitMultByMFunction(unsigned int index) final;
    virtual void analyzeMatrix() final;
    virtual void convertMatrix() final;
    
  private:
    std::vector<unsigned long> maxRowLengths;
  };
}

#endif
