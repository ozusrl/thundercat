#!/bin/bash

PWD=`pwd`
SUFFIX=tags/RELEASE_350/final/
DESTINATION=src_350
BUILD_DIR=build_350

svn co $REVISION http://llvm.org/svn/llvm-project/llvm/$SUFFIX $DESTINATION

cd $DESTINATION/tools
svn co $REVISION http://llvm.org/svn/llvm-project/cfe/$SUFFIX clang
cd ../..

cd $DESTINATION/tools
svn co $REVISION http://llvm.org/svn/llvm-project/lldb/$SUFFIX lldb
cd ../..

cd $DESTINATION/tools/clang/tools
svn co $REVISION http://llvm.org/svn/llvm-project/clang-tools-extra/$SUFFIX extra
cd ../../../..

cd $DESTINATION/projects
svn co $REVISION http://llvm.org/svn/llvm-project/compiler-rt/$SUFFIX compiler-rt
cd ../..

read -n1 -r -p "SVN checkout complete. Press a key to apply the patch." key

### Apply SpMVlib-specific changes
cd $DESTINATION
patch -p0 -i ../llvm_350_thundercat_patch.diff
cd ..

read -n1 -r -p "Patch applied. Press a key to start the build process." key

### Compilation
mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_CXX1Y=ON ../$DESTINATION
ninja
cd ..
ln -s $DESTINATION src
ln -s $BUILD_DIR build

### PATH
echo "Add the following line to your bash profile."
echo "export PATH=\$PATH:$PWD/build/bin"

