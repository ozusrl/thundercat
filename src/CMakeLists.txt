cmake_minimum_required(VERSION 2.8.8)
project(thundercat)

message(STATUS "System is " ${CMAKE_SYSTEM_NAME})
message(STATUS "Compiler is " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "Source dir is " ${CMAKE_SOURCE_DIR})
message(STATUS "Source dir is " ${CMAKE_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../modules/")

include_directories(${CMAKE_SOURCE_DIR}/../asmjit/src)
link_directories(${CMAKE_SOURCE_DIR}/../asmjit/build)

set(MKL_ROOT_DIR $ENV{MKLROOT})

if (MKL_ROOT_DIR)
    message(STATUS "Found MKL at " $ENV{MKLROOT})
    add_definitions(-DMKL_EXISTS)
    include_directories($MKL_ROOT_DIR/include)
    if (APPLE)
        link_directories($ENV{DYLD_LIBRARY_PATH})
    else()
        link_directories($ENV{LD_LIBRARY_PATH})
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64" )
endif()

find_package(OpenMP)
if (OPENMP_FOUND)
    add_definitions(-DOPENMP_EXISTS)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    message(STATUS "System is " ${CMAKE_SYSTEM_NAME})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fno-rtti -std=c++14" )

message(STATUS "CXX Flags: " ${CMAKE_CXX_FLAGS})
message(STATUS "Linker Flags: " ${CMAKE_EXE_LINKER_FLAGS})
set(dir ${CMAKE_CURRENT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}")

set(SOURCE_FILES
                 csrByNZ.cpp
                 csrWithGOTO.cpp
                 duffsDevice.cpp
                 genOski.cpp
                 main.cpp
                 matrix.cpp
                 method.cpp
                 mkl.cpp
                 plaincsr.cpp
                 profiler.cpp
                 rowPattern.cpp
                 svmAnalyzer.cpp
                 unfolding.cpp
                 unrollingWithGOTO.cpp
)

set(HEADER_FILES
                 matrix.h
                 method.h
                 profiler.h
                 svmAnalyzer.h
)

add_executable(thundercat ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(thundercat asmjit)

if (MKL_ROOT_DIR)
    target_link_libraries(thundercat
        mkl_core
        mkl_intel_thread
        mkl_intel_lp64
        iomp5
        pthread
        m
        dl
    )
endif()
