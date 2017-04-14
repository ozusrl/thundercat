#!/bin/bash

if [ -z ${MATRICES+x} ]; then
    echo "Set MATRICES variable to the matrices folder."
    exit 1
fi
   
while read matrixName
do
    echo -n $matrixName" "
    ../build/thundercat $MATRICES/$matrixName/$matrixName PlainCSR -matrix_stats | tail -n 1
done < matrixNames.txt
