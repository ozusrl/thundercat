#ifndef THUNDERCAT_CSR5_H
#define THUNDERCAT_CSR5_H

#include "method.h"
#include "anonymouslib_avx2.h"

namespace thundercat {
    class Csr5avx2 : public SpmvMethod {

    public:

        static const std::string name;

        virtual void init(unsigned int numThreads);

        virtual ~Csr5avx2();

        virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);

        virtual void spmv(double* __restrict v, double* __restrict w);

    private:
        unsigned int mNumThreads;
        bool isXSet;
        std::unique_ptr<anonymouslibHandle<int, unsigned int, VALUE_TYPE>> underlying;
        int * csrRowPtr;
        int * csrColIdx;
        VALUE_TYPE * csrVal;
        std::unique_ptr<CSRMatrix<VALUE_TYPE>> csr;

    };
}


#endif //THUNDERCAT_CSR5_H
