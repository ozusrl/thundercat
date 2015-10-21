#!/bin/bash

testMultiThread() {
    local numOfThreads=$1 
    ./runStaticSpMVlibTests.sh "stencil" -num_threads $numOfThreads
    ./runStaticSpMVlibTests.sh "genOSKI44" -num_threads $numOfThreads
    ./runStaticSpMVlibTests.sh "genOSKI55" -num_threads $numOfThreads
    ./runStaticSpMVlibTests.sh "unfoldingWithDistinctValues" -num_threads $numOfThreads
    ./runStaticSpMVlibTests.sh "CSRbyNZ" -num_threads $numOfThreads
    ./runStaticMKLTests.sh $numOfThreads 
    echo "$numOfThreads""-thread test on $HOSTNAME has finished." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
}

#testMultiThread 1
testMultiThread 4

rm -rf "$HOSTNAME"_static_spMVlib_all
mkdir "$HOSTNAME"_static_spMVlib_all
$(mv "$HOSTNAME".spMVgen.*csv "$HOSTNAME"_static_spMVlib_all)

#echo "Test on $HOSTNAME has finished." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
