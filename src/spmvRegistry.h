#ifndef SPMV_BENCHMARKING_SPMV_REGISTRY_H
#define SPMV_BENCHMARKING_SPMV_REGISTRY_H

#include <list>
#include <string>
#include <map>
#include "method.h"

#define SpmvMethod thundercat::SpMVMethod

class SpmvMethodCreator {
public:
    virtual std::unique_ptr<SpmvMethod> getMethod() = 0;
};

class SpmvMethodRegistry {
public:

    static SpmvMethodRegistry &instance();


    void add(SpmvMethodCreator *creator, std::string name);

    std::unique_ptr<SpmvMethod> getMethod(std::string name);


private:
    std::map<std::string, SpmvMethodCreator *> registry;

    SpmvMethodRegistry() : registry() {};
    SpmvMethodRegistry(SpmvMethodRegistry const &) {};
    void operator=(SpmvMethodRegistry const &) {};
};


template<class TSpmvMethod>
class SpmvMethodCreatorImpl : public SpmvMethodCreator {
public:
    SpmvMethodCreatorImpl(std::string name);
    std::unique_ptr<SpmvMethod> getMethod();
};

template<class TSpmvMethod>
SpmvMethodCreatorImpl<TSpmvMethod>::SpmvMethodCreatorImpl(std::string name) {
  SpmvMethodRegistry &factory = SpmvMethodRegistry::instance();
  factory.add(this, TSpmvMethod::name);
}

template<class TSpmvMethod>
std::unique_ptr<SpmvMethod> SpmvMethodCreatorImpl<TSpmvMethod>::getMethod() {
  return std::unique_ptr<SpmvMethod>(new TSpmvMethod());
}

#define REGISTER_METHOD(CLASSNAME) \
    namespace { \
        static SpmvMethodCreatorImpl<CLASSNAME> \
        CLASSNAME##_creator( CLASSNAME :: name); \
    };

#endif //SPMV_BENCHMARKING_SPMV_REGISTRY_H
