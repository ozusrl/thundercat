#ifndef _METHOD_H_
#define _METHOD_H_

#include "matrix.h"
#include <unordered_map>
#include <iostream>
#include "asmjit/asmjit.h"

namespace spMVgen {
  // multByM(v, w, rows, cols, vals)
  typedef void(*MultByMFun)(double*, double*, int*, int*, double*);
  
  class SpMVMethod {
  public:
    SpMVMethod(Matrix *csrMatrix);

    virtual ~SpMVMethod();
    
    virtual bool isSpecializer();
    
    virtual void emitCode();
    
    virtual std::vector<MultByMFun> getMultByMFunctions() = 0;
    
    virtual Matrix* getCustomMatrix() final;
    
    virtual void processMatrix() final;
  
  protected:
    virtual void analyzeMatrix() = 0;
    virtual void convertMatrix() = 0;
    
    std::vector<MatrixStripeInfo> *stripeInfos;
    Matrix *csrMatrix;
    Matrix *matrix;
  };
  
  ///
  /// MKL
  ///
  class MKL: public SpMVMethod {
  public:
    MKL(Matrix *csrMatrix);

    virtual std::vector<MultByMFun> getMultByMFunctions();

  protected:
    virtual void analyzeMatrix();
    virtual void convertMatrix();
  };
  
  ///
  /// PlainCSR
  ///
  class PlainCSR: public SpMVMethod {
  public:
    PlainCSR(Matrix *csrMatrix);
    
    virtual std::vector<MultByMFun> getMultByMFunctions();
    
  protected:
    virtual void analyzeMatrix();
    virtual void convertMatrix();
  };


  ///
  /// Specializer
  ///
  class Specializer : public SpMVMethod {
  public:
    Specializer(Matrix *csrMatrix);
    
    virtual bool isSpecializer() final;
    
    virtual void emitCode() final;
  
    virtual std::vector<MultByMFun> getMultByMFunctions() final;

    std::vector<asmjit::CodeHolder*> *getCodeHolders();
    
  protected:
    virtual void emitMultByMFunction(unsigned int index) = 0;
    
    std::vector<asmjit::CodeHolder*> codeHolders;
    
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
  public:
    CSRbyNZ(Matrix *csrMatrix);
    
    virtual void emitMultByMFunction(unsigned int index);
    
  protected:
    virtual void analyzeMatrix();
    virtual void convertMatrix();

  private:
    std::vector<NZtoRowMap> rowByNZLists;
  };
  
//  ///
//  /// Gen OSKI
//  ///
//  class GenOSKI: public Specializer {
//  private:
//    unsigned int b_r, b_c;
//
//  public:
//    GenOSKI(Matrix *csrMatrix, unsigned b_r, unsigned b_c);
//    
//    virtual void emitMultByMFunction(unsigned int index);
//    
//  protected:
//    GenOSKIAnalyzer analyzer;
//  };
//
//  ///
//  /// Stencil
//  ///
//  class Stencil: public Specializer {
//  public:
//    Stencil(Matrix *csrMatrix);
//
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    StencilAnalyzer analyzer;
//  };
//  
//
//  ///
//  /// Unfolding
//  ///
//  class Unfolding: public Specializer {
//  public:
//    Unfolding(Matrix *csrMatrix);
//
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    UnfoldingAnalyzer analyzer;
//  };
//
//  ///
//  /// UnrollingWithGOTO
//  ///
//  class UnrollingWithGOTO: public Specializer {
//  public:
//    UnrollingWithGOTO(Matrix *csrMatrix);
//
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    UnrollingWithGOTOAnalyzer analyzer;
//  };
//  
//  ///
//  /// CSRWithGOTO
//  ///
//  class CSRWithGOTO: public Specializer {
//  public:
//    CSRWithGOTO(Matrix *csrMatrix);
//    
//    virtual void emitMultByMFunction(unsigned int index);
//
//  protected:
//    CSRWithGOTOAnalyzer analyzer;
//  };
}

#endif
