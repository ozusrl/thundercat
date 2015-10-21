#!/bin/bash

testMethod() {
    local methodName=$1
    local methodParam1=$2
    local methodParam2=$3
    ./generateMethodFolders.sh "$methodName" "$methodParam1" "$methodParam2"
    ./compilePythonGeneratedCodes.sh "$methodName" "$methodParam1" "$methodParam2"
    ./runPythonGeneratedCodesForSpeed.sh "$methodName" "$methodParam1" "$methodParam2"
    ./makeCSVForPythonGeneratedCodes.sh "$methodName" "$methodParam1" "$methodParam2" > $HOSTNAME."$methodName""$methodParam1""$methodParam2".csv
}

testMethod CSRbyNZ
testMethod stencil
testMethod unfolding
#testMethod unrolling 1
#testMethod unrolling 2
#testMethod unrolling 3
#testMethod unrolling 4
#testMethod unrolling 5
#testMethod genOSKI 2 2
#testMethod genOSKI 3 2
#testMethod genOSKI 2 3
#testMethod genOSKI 3 3

rm -rf "$HOSTNAME"_clang_all
mkdir "$HOSTNAME"_clang_all
$(mv "$HOSTNAME."*csv "$HOSTNAME"_clang_all)
