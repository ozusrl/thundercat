#ifndef _THUNDERCAT_ZZ_CRS_HPP
#include "method.h"
#include "ZZ_CRS.hpp"
#include "Hilbert.hpp"
#include "HTS.hpp"

namespace thundercat {

  template <typename UNDERLYING>
  class YzelmanMethod : public SpmvMethod {

  public:
      YzelmanMethod() {};

      virtual void init(unsigned int numThreads) {};

      virtual ~YzelmanMethod() {};

      virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix) {

        std::vector<Triplet<VALUE_TYPE>> triplets;

        auto elems = matrix.getElements();


        for (auto elem = elems.begin() ; elem != elems.end(); ++elem)
          triplets.push_back(Triplet<VALUE_TYPE>(elem->rowIndex, elem->colIndex, elem->value));


        underlying = std::move(createUnderlying(triplets, matrix.M, matrix.N, 0));
      };

      virtual void spmv(double* __restrict v, double* __restrict w) {
        underlying->zax(v,w);
      };

      virtual std::unique_ptr<UNDERLYING> createUnderlying(
          std::vector<Triplet<VALUE_TYPE> > &input,
          int m,
          int n,
          VALUE_TYPE zero) = 0;

  private:
      std::unique_ptr<UNDERLYING> underlying;
  };

  class ZzCrs: public YzelmanMethod<ZZ_CRS<VALUE_TYPE>> {

  protected:
      virtual std::unique_ptr<ZZ_CRS<VALUE_TYPE>> createUnderlying(
          std::vector<Triplet<VALUE_TYPE>> &input,
          int m,
          int n,
          VALUE_TYPE zero);
  };

  class THilbert : public YzelmanMethod<Hilbert<VALUE_TYPE>> {

  protected:
      virtual std::unique_ptr<Hilbert<VALUE_TYPE>> createUnderlying(
          std::vector<Triplet<VALUE_TYPE>> &input,
          int m,
          int n,
          VALUE_TYPE zero);
  };

    class Hts : public YzelmanMethod<HTS<VALUE_TYPE>> {

    protected:
        virtual std::unique_ptr<HTS<VALUE_TYPE>> createUnderlying(
            std::vector<Triplet<VALUE_TYPE>> &input,
            int m,
            int n,
            VALUE_TYPE zero);
    };
}

#endif