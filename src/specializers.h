#ifndef THUNDERCAT_SPECIALIZERS_H
#define THUNDERCAT_SPECIALIZERS_H

#include "method.h"
#include "mmmatrix.hpp"

namespace thundercat {

  // multByM(v, w, rows, cols, vals)
  typedef void(*MultByMFun)(double *, double *, int *, int *, double *);

  ///
  /// Specializer
  ///
  class Specializer : public CsrSpmvMethod {
  public:
    virtual void init(unsigned int numThreads) final;

    virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);

    virtual std::vector<asmjit::CodeHolder*> *getCodeHolders() final;

    virtual void spmv(double* __restrict v, double* __restrict w) final;

  protected:
    virtual void emitMultByMFunction(unsigned int index) = 0;

    std::vector<asmjit::CodeHolder*> codeHolders;

    std::vector<MultByMFun> functions;
    std::unique_ptr<CSRMatrix<VALUE_TYPE>> matrix;

  private:
    virtual void emitCode() final;
    asmjit::JitRuntime rt;
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
    virtual void analyzeMatrix(CSRMatrix<VALUE_TYPE> &csrMatrix,
                               std::vector<MatrixStripeInfo> stripeInfos,
                               std::vector<NZtoRowMap> &rowByNZLists) final;
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

  class GenOSKI33: public GenOSKI {
  public:
    GenOSKI33();
  };

  class GenOSKI44: public GenOSKI {
  public:
    GenOSKI44();
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


#endif //THUNDERCAT_SPECIALIZERS_H
