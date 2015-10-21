PARSER_DIR=../compiler
JAVACLASSPATH=$(PARSER_DIR)
UNAME_S := $(shell uname -s)
ICC_PATH=$(shell whereis icc)
ifeq ($(UNAME_S),Darwin)
  CLANG=clang++
  VEND=Intel
else
  CLANG=icc
  VEND=$(shell cat /proc/cpuinfo | grep -m 1 vendor | awk -F': ' '{print $$2}')
endif
ifeq ($(ICC_PATH),icc:)
  CLANG=clang++
endif
ifeq ($(VEND),AuthenticAMD)
  CLANG=g++
endif
OPT_LEVEL=-O3 -fopenmp

CXXFLAGS=`llvm-config --cxxflags` -I`llvm-config --src-root` -I`llvm-config --obj-root`/lib/Target/X86/
LDFLAGS=`llvm-config --ldflags`
C11FLAGS=-std=c++11

ifeq ($(UNAME_S),Darwin)
    C11FLAGS += -stdlib=libc++
    LDFLAGS += -stdlib=libc++
else
    ifneq ($(CLANG),g++)
      LDFLAGS += -lmkl_intel_lp64 -lmkl_core -lmkl_intel_thread -liomp5
    else
      LDFLAGS += -lmkl_intel_lp64 -lmkl_core -lmkl_gnu_thread -fopenmp
    endif
endif

LIBS=`llvm-config --libs all` `llvm-config --system-libs`
BUILD_DIR=build
TARGET=spMVgen
HEADERS=$(wildcard *.h)
SOURCE=$(wildcard *.cpp)
BARE_OBJS:=$(SOURCE:.cpp=.o)

OBJS:=$(foreach obj, $(BARE_OBJS), $(BUILD_DIR)/$(obj))

all: $(TARGET)

$(TARGET): $(BUILD_DIR) $(OBJS)
	$(CLANG) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LIBS) 

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	$(CLANG) $(OPT_LEVEL) -c $(CXXFLAGS) $(C11FLAGS) $< -o $@

