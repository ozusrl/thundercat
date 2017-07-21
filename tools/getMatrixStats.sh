#!/bin/bash

if [ -z ${MATRICES+x} ]; then
    echo "Set MATRICES variable to the matrices folder."
    exit 1
fi

source /opt/intel/bin/compilervars.sh intel64

while read line
do
    IFS=' ' read -r -a info <<< "$line"
    groupName=${info[0]}
    matrixName=${info[1]}

    echo -n $groupName" "$matrixName" "

    ../build/thundercat $MATRICES/$groupName/$matrixName/$matrixName PlainCSR -matrix_stats | tail -n 1
done < matrixNames.txt
