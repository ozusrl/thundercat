cmake_minimum_required(VERSION 3.8)
project(spmv_benchmark)

message(STATUS "System is " ${CMAKE_SYSTEM_NAME})
message(STATUS "Compiler is " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "Source dir is " ${CMAKE_SOURCE_DIR})
message(STATUS "Source dir is " ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fno-rtti -std=c++14")

message(STATUS "CXX Flags: " ${CMAKE_CXX_FLAGS})
message(STATUS "Linker Flags: " ${CMAKE_EXE_LINKER_FLAGS})
set(dir ${CMAKE_CURRENT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}")


###################################
### Setup source & header files ###
###################################

set(SOURCE_FILES
        method.cpp
        incrementalCSR.cpp
        profiler.cpp
        plaincsr.cpp
        main.cpp
        spmvRegistry.cpp
        parse_options.cpp
        newmethod.cpp
        )

set(HEADER_FILES
        incrementalCSR.hpp
        method.h
        profiler.h
        spmvRegistry.h
        parse_options.h
        newmethod.h
        )

if (CODEGEN)
    set(SOURCE_FILES
            ${SOURCE_FILES}
            csrByNZ.cpp
            csrLenWithGOTO.cpp
            csrWithGOTO.cpp
            genOski.cpp
            rowPattern.cpp
            specializers.cpp
            unfolding.cpp
            unrollingWithGOTO.cpp
            )

    set(HEADER_FILES ${HEADER_FILES} specializers.h)
endif()

##
## Yzelman
##
if (YZELMAN)
    set(SOURCE_FILES
            ${SOURCE_FILES}
            yzelman.cpp
            ${YZELMAN}/src/Matrix2HilbertCoordinates.cpp
            )
    set(HEADER_FILES
            ${HEADER_FILES}
            yzelman.hpp
            )
    include_directories(${YZELMAN}/include)
endif ()

##
## OpenMP
##
find_package(OpenMP)
if (OPENMP_FOUND)
    add_definitions(-DOPENMP_EXISTS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    message(STATUS "System is " ${CMAKE_SYSTEM_NAME})
endif ()


##
## CUDA
##
find_package(CUDA)
if (CUDA_FOUND)
    enable_language(CUDA)
    CUDA_SELECT_NVCC_ARCH_FLAGS(ARCH_FLAGS Auto)
    LIST(APPEND CUDA_NVCC_FLAGS ${ARCH_FLAGS} -std=c++11)
    set(SOURCE_FILES ${SOURCE_FILES} cusparse.cpp cusparseAdaptor.cu)
    set(HEADER_FILES ${HEADER_FILES} cusparse.hpp cusparseAdaptor.hu)

endif()

##
## ViennaCL
##
if (VIENNACL)
    set(SOURCE_FILES ${SOURCE_FILES} viennacl.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} viennacl.hpp)
    set(SOURCE_FILES ${SOURCE_FILES} viennaclAdapter.hpp)
    if (CUDA_FOUND)
        message("-- ViennaCL: Found CUDA: using CUDA Backend")
        add_definitions("-DVIENNACL_WITH_CUDA")
        set(SOURCE_FILES ${SOURCE_FILES} viennaclAdapter.cu)
    else()
        if(OPENMP_FOUND)
            message("-- ViennaCL: Found OpenMP: using OpenMP Backend")
            add_definitions("-DVIENNACL_WITH_OPENMP")
        endif()
        set(SOURCE_FILES ${SOURCE_FILES} viennaclAdapter.cpp)
    endif()
endif()

##
## CUSP
##

if (CUSP)
    if(CUDA_FOUND)
        set(SOURCE_FILES ${SOURCE_FILES} cusp.cpp cuspAdapter.cu)
        set(HEADER_FILES ${HEADER_FILES} cusp.hpp cuspAdapter.hu)
        include_directories(${CUSP})

    else()
        message(FATAL_ERROR "CUSP enabled but no CUDA found")
    endif()
endif()

##
## MKL
##
set(MKL_ROOT_DIR $ENV{MKLROOT})
if (MKL_ROOT_DIR)
    set(SOURCE_FILES ${SOURCE_FILES} mkl.cpp)
    set(HEADER_FILES ${HEADER_FILES} mkl.hpp)
endif ()


#########################
### Define executable ###
#########################

add_executable(spmv_benchmark ${SOURCE_FILES} ${HEADER_FILES})


#########################
### Library discovery ###
#########################

##
## asmjit library
##
include_directories(${CMAKE_SOURCE_DIR}/../asmjit/src)
find_library(ASMJIT
        asmjit
        PATHS ${CMAKE_SOURCE_DIR}/../asmjit/build
        )
target_link_libraries(spmv_benchmark ${ASMJIT})

##
## MKL
##
if (MKL_ROOT_DIR)
    message(STATUS "Found MKL at " $ENV{MKLROOT})
    add_definitions(-DMKL_EXISTS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    include_directories(${MKL_ROOT_DIR}/include)
    find_library(MKL_CORE
            mkl_core
            PATHS ${MKL_ROOT_DIR}/lib ${MKL_ROOT_DIR}/lib/intel64
            )
    find_library(MKL_INTEL_THREAD
            mkl_intel_thread
            PATHS ${MKL_ROOT_DIR}/lib ${MKL_ROOT_DIR}/lib/intel64
            )
    find_library(MKL_INTEL_LP64
            mkl_intel_lp64
            PATHS ${MKL_ROOT_DIR}/lib ${MKL_ROOT_DIR}/lib/intel64
            )
    find_library(MKL_INTEL_LP64
            iomp5
            PATHS ${MKL_ROOT_DIR}/../lib ${MKL_ROOT_DIR}/../lib/intel64
            )
    target_link_libraries(spmv_benchmark
            ${MKL_CORE}
            ${MKL_INTEL_THREAD}
            ${MKL_INTEL_LP64}
            ${IOMP5}
            pthread
            m
            dl
            )
endif ()


##
## MMMatrixIO library
##
find_library(MMMatrixIO
        mmmatrixio
        PATHS ${CMAKE_SOURCE_DIR}/../lib/mmmatrixio
        )
target_link_libraries(spmv_benchmark ${MMMatrixIO})
target_include_directories(spmv_benchmark PUBLIC ${CMAKE_SOURCE_DIR}/../MMMatrixIO/src)

#
# docopt library
#
include_directories(${CMAKE_SOURCE_DIR}/../docopt.cpp)
find_library(docopt
        docopt
        PATHS ${CMAKE_SOURCE_DIR}/../lib/docopt.cpp
        )
target_link_libraries(spmv_benchmark ${docopt})

target_link_libraries(spmv_benchmark ${CUDA_cusparse_LIBRARY} )

#
# ViennaCL library
#
if (VIENNACL)
    find_library(VIENNACL_LIB
            viennacl
            PATHS ${VIENNACL}/build/libviennacl
            )
    target_link_libraries(spmv_benchmark ${VIENNACL_LIB})
    target_include_directories(spmv_benchmark PUBLIC ${VIENNACL}/libviennacl/include ${VIENNACL})
endif()


