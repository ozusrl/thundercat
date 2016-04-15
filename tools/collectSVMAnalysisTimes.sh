#!/bin/bash

if [ -z ${MATRICES+x} ]; then
    echo "Set MATRICES variable to the matrices folder."
    exit 1
fi

while read matrixName
do
    echo -n $matrixName" "
    cd ..
    ./build/spMVgen $MATRICES/$matrixName PlainCSR -matrix_stats
    cd tools
    
done < matrixNames.txt
    
echo "Test on $HOSTNAME has finished." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
