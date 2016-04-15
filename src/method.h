#ifndef _METHOD_H_
#define _METHOD_H_

#include "matrix.h"
#include <unordered_map>
#include <iostream>
#include "stencilAnalyzer.h"
#include "csrByNZAnalyzer.h"
#include "unfoldingAnalyzer.h"
#include "unrollingWithGOTOAnalyzer.h"
#include "genOskiAnalyzer.h"
#include "codeEmitter.h"

#define MAIN_FUNCTION_NAME "multByM"

namespace spMVgen {
  // multByM(v, w, rows, cols, vals)
  typedef void(*MultByMFun)(double*, double*, int*, int*, double*);
  
  class SpMVMethod {
  public:
    SpMVMethod(Matrix *csrMatrix);

    virtual ~SpMVMethod();
    
    Matrix* getMatrix();

    Matrix* getCSRMatrix();
    
    bool hasNonEmptyMatrix();

    void setMCStreamer(llvm::MCStreamer *Str);

    virtual void dumpAssemblyText() = 0;

    void dumpMultByMFunctions();

  protected:
    virtual Matrix* getMatrixForGeneration() = 0;

    llvm::MCStreamer *Str;
    Matrix *csrMatrix;
    Matrix *matrix;
  };
  
  ///
  /// Gen OSKI
  ///
  class GenOSKI: public SpMVMethod {
  private:
    unsigned int b_r, b_c;

  public:
    GenOSKI(Matrix *csrMatrix, unsigned b_r, unsigned b_c);
    
    virtual void dumpAssemblyText();
    
  protected:
    Matrix* getMatrixForGeneration();
    GenOSKIAnalyzer analyzer;
  };

  ///
  /// CSRbyNZ
  ///
  class CSRbyNZ: public SpMVMethod {
  public:
    CSRbyNZ(Matrix *csrMatrix);

    virtual void dumpAssemblyText();

  protected:
    Matrix* getMatrixForGeneration();
    CSRbyNZAnalyzer analyzer;
  };
  
  ///
  /// Stencil
  ///
  class Stencil: public SpMVMethod {
  public:
    Stencil(Matrix *csrMatrix);

    virtual void dumpAssemblyText();

  protected:
    Matrix* getMatrixForGeneration();
    StencilAnalyzer analyzer;
  };
  

  ///
  /// Unfolding
  ///
  class Unfolding: public SpMVMethod {
  public:
    Unfolding(Matrix *csrMatrix);

    virtual void dumpAssemblyText();
  protected:
    Matrix* getMatrixForGeneration();
    
    UnfoldingAnalyzer analyzer;
  };

  ///
  /// UnrollingWithGOTO
  ///
  class UnrollingWithGOTO: public SpMVMethod {
  public:
    UnrollingWithGOTO(Matrix *csrMatrix);

    virtual void dumpAssemblyText();
  protected:
    Matrix* getMatrixForGeneration();
    UnrollingWithGOTOAnalyzer analyzer;
  };
  
  ///
  /// MKL
  ///
  class MKL: public SpMVMethod {
  public:
    MKL(Matrix *csrMatrix);
    
    virtual void dumpAssemblyText();
    
    void setNumOfThreads(unsigned int num);
    
    std::vector<MultByMFun> getMultByMFunctions();
    
  protected:
    Matrix* getMatrixForGeneration();
  };


  ///
  /// PlainCSR
  ///
  class PlainCSR: public SpMVMethod {
  public:
    PlainCSR(Matrix *csrMatrix);
    
    virtual void dumpAssemblyText();
    
    std::vector<MultByMFun> getMultByMFunctions();
    
  protected:
    Matrix* getMatrixForGeneration();
  };
}

#endif
