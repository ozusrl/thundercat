#!/bin/bash

testMultiThread() {
    local numOfThreads=$1 
    ./runSpMVlibTests.sh "CSRbyNZ" -num_threads $numOfThreads
    ./runSpMVlibTests.sh "stencil" -num_threads $numOfThreads
    ./runSpMVlibTests.sh "genOSKI44" -num_threads $numOfThreads
    ./runSpMVlibTests.sh "genOSKI55" -num_threads $numOfThreads
    ./runSpMVlibTests.sh "unfolding" -num_threads $numOfThreads
    ./runSpMVlibTests.sh "MKL" -num_threads $numOfThreads
    ./runSpMVlibTests.sh "PlainCSR" -num_threads $numOfThreads
}

#testMultiThread 1
testMultiThread 6

rm -rf "$HOSTNAME"_dynamic_spMVlib_all
mkdir "$HOSTNAME"_dynamic_spMVlib_all
$(mv "$HOSTNAME".spMVgen.dynamic.*csv "$HOSTNAME"_dynamic_spMVlib_all)

echo "Test on $HOSTNAME has finished." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
