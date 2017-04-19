#ifndef _METHOD_H_
#define _METHOD_H_

#include "matrix.h"
#include <unordered_map>
#include <iostream>
#include "asmjit/asmjit.h"

namespace thundercat {
  // multByM(v, w, rows, cols, vals)
  typedef void(*MultByMFun)(double*, double*, int*, int*, double*);
  
  class SpMVMethod {
  public:
    virtual void init(Matrix *csrMatrix, unsigned int numThreads);

    virtual ~SpMVMethod();
    
    virtual bool isSpecializer();
    
    virtual void emitCode();
    
    virtual Matrix* getMethodSpecificMatrix() final;
    
    virtual void processMatrix() final;
  
    virtual void spmv(double *v, double *w) = 0;
    
  protected:
    virtual void analyzeMatrix();
    virtual void convertMatrix();
    
    std::vector<MatrixStripeInfo> *stripeInfos;
    Matrix *csrMatrix;
    Matrix *matrix;
    unsigned int numPartitions;
  };
  
  ///
  /// MKL
  ///
  class MKL: public SpMVMethod {
  public:
    virtual void init(Matrix *csrMatrix, unsigned int numThreads) final;
    
    virtual void spmv(double *v, double *w) final;
  };
  
  ///
  /// PlainCSR
  ///
  class PlainCSR: public SpMVMethod {
  public:
    virtual void spmv(double *v, double *w) final;
  };

  class PlainCSR2: public SpMVMethod {
  public:
    virtual void spmv(double *v, double *w) final;
  };

  class PlainCSR4: public SpMVMethod {
  public:
    virtual void spmv(double *v, double *w) final;
  };

  class PlainCSR8: public SpMVMethod {
  public:
    virtual void spmv(double *v, double *w) final;
  };


  ///
  /// Specializer
  ///
  class Specializer : public SpMVMethod {
  public:
    virtual void init(Matrix *csrMatrix, unsigned int numThreads) final;
    
    virtual bool isSpecializer() final;
    
    virtual void emitCode() final;
  
    virtual std::vector<asmjit::CodeHolder*> *getCodeHolders() final;

    virtual void spmv(double *v, double *w) final;
    
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
  class RowByNZ {
  public:
    std::vector<int> *getRowIndices();
    void addRowIndex(int index);
    
  private:
    std::vector<int> rowIndices;
  };
  
  // To keep the map keys in ascending order, using the "greater" comparator.
  typedef std::map<unsigned long, RowByNZ, std::greater<unsigned long> > NZtoRowMap;
  
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
}

#endif
