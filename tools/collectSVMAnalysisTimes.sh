#!/bin/bash

while read matrixName
do
    echo -n $matrixName" "
    cd ..
    ./spMVgen matrices/$matrixName PlainCSR -matrix_stats
    cd tools
    
done < matrixNames.txt
    
echo "Test on $HOSTNAME has finished." | mail -s "Test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
