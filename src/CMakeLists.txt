cmake_minimum_required(VERSION 2.8.8)
project(thundercat)

message(STATUS "System is " ${CMAKE_SYSTEM_NAME})
message(STATUS "Compiler is " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "Source dir is " ${CMAKE_SOURCE_DIR})
message(STATUS "Source dir is " ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fno-rtti -std=c++14" )

message(STATUS "CXX Flags: " ${CMAKE_CXX_FLAGS})
message(STATUS "Linker Flags: " ${CMAKE_EXE_LINKER_FLAGS})
set(dir ${CMAKE_CURRENT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}")


set(SOURCE_FILES
                 csrByNZ.cpp
                 csrLenWithGOTO.cpp
                 csrWithGOTO.cpp
#                 duffsDevice.cpp
#                 duffsDeviceLCSR.cpp
                 genOski.cpp
                 incrementalCSR.cpp
#                 main.cpp
                 method.cpp
#                 mkl.cpp
                 plaincsr.cpp
                 profiler.cpp
                 rowPattern.cpp
#                 svmAnalyzer.cpp
                 unfolding.cpp
                 unrollingWithGOTO.cpp
                 main2.cpp
                 spmvRegistry.cpp
                 parse_options.cpp
                 zzCsr.cpp
)

set(HEADER_FILES
#                 duffsDeviceCSRDD.hpp
#                 duffsDeviceCompressed.hpp
                 incrementalCSR.hpp
                 method.h
                 profiler.h
#                 svmAnalyzer.h
                 spmvRegistry.h
                 parse_options.h
                 zzCsr.hpp
)

add_executable(thundercat ${SOURCE_FILES} ${HEADER_FILES})

##
## asmjit library
##
include_directories(${CMAKE_SOURCE_DIR}/../asmjit/src)
find_library(ASMJIT
  asmjit
  PATHS ${CMAKE_SOURCE_DIR}/../asmjit/build
)
target_link_libraries(thundercat ${ASMJIT})


##
## MKL
##
set(MKL_ROOT_DIR $ENV{MKLROOT})
if (MKL_ROOT_DIR)
  message(STATUS "Found MKL at " $ENV{MKLROOT})
  add_definitions(-DMKL_EXISTS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64" )
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
  target_link_libraries(thundercat
    ${MKL_CORE}
    ${MKL_INTEL_THREAD}
    ${MKL_INTEL_LP64}
    ${IOMP5}
    pthread
    m
    dl
  )
endif()


##
## OpenMP
##
find_package(OpenMP)
if (OPENMP_FOUND)
  add_definitions(-DOPENMP_EXISTS)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  message(STATUS "System is " ${CMAKE_SYSTEM_NAME})
endif()

##
## MMMatrixIO library
##
find_library(MMMatrixIO
        mmmatrixio
        PATHS ${CMAKE_SOURCE_DIR}/../lib/mmmatrixio
        )
target_link_libraries(thundercat ${MMMatrixIO})
target_include_directories(thundercat PUBLIC ${CMAKE_SOURCE_DIR}/../MMMatrixIO/src)

#
# docopt library
#
include_directories(${CMAKE_SOURCE_DIR}/../docopt.cpp)
find_library(docopt
        docopt
        PATHS ${CMAKE_SOURCE_DIR}/../lib/docopt.cpp
        )
target_link_libraries(thundercat ${docopt})


##
## Yzelman
##
include_directories(${CMAKE_SOURCE_DIR}/../yzelman/include)
find_library(Yzelman
        sparselib
        PATHS ${CMAKE_SOURCE_DIR}/../yzelman/lib
        )
target_link_libraries(thundercat ${docopt})
