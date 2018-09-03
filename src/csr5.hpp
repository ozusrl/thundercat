#ifndef THUNDERCAT_CSR5_H
#define THUNDERCAT_CSR5_H

#endif //THUNDERCAT_CSR5_H

#include "method.h"
#include "anonymouslib_avx2.h"


namespace thundercat {
    class Csr5 : public SpmvMethod {

    public:

        static const std::string name;

        virtual void init(unsigned int numThreads);

        virtual ~Csr5();

        virtual void preprocess(MMMatrix<VALUE_TYPE>& matrix);

        virtual void spmv(double* __restrict v, double* __restrict w);

    private:
        unsigned int mNumThreads;
        bool isXSet;
        std::unique_ptr<anonymouslibHandle<int, unsigned int, VALUE_TYPE>> underlying;

    };
}