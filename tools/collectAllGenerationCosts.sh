#!/bin/bash

testMultiThread() {
    local numOfThreads=$1 
    ./collectGenerationCosts.sh "CSRbyNZ" -num_threads $numOfThreads
    ./collectGenerationCosts.sh "stencil" -num_threads $numOfThreads
    ./collectGenerationCosts.sh "genOSKI44" -num_threads $numOfThreads
    ./collectGenerationCosts.sh "genOSKI55" -num_threads $numOfThreads
    ./collectGenerationCosts.sh "unfolding" -num_threads $numOfThreads
    #echo "$numOfThreads""-thread test on $HOSTNAME has finished." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
}

testMultiThread 1
testMultiThread 6

echo "Test on $HOSTNAME." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
