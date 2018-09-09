#ifndef THUNDERCAT_CSR5AVX512_H
#define THUNDERCAT_CSR5AVX512_H

#include "method.h"
#include "anonymouslib_cuda.h"

namespace thundercat {
    class Csr5avx512 : public SpmvMethod {

    public:

        static const std::string name;

        virtual void init(unsigned int numThreads);

        virtual ~Csr5avx512();

        virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);

        virtual void spmv(double* __restrict v, double* __restrict w);

    private:
        unsigned int mNumThreads;
        bool isXSet;
        std::unique_ptr<anonymouslibHandle<int, unsigned int, VALUE_TYPE>> underlying;
        int * csrRowPtr;
        int * csrColIdx;
        VALUE_TYPE * csrVal;
    };
}


#endif //THUNDERCAT_CSR5AVX512_H
