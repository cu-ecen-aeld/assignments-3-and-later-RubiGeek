#!/bin/sh
NUM_ARGS=2

if [ $# -ne ${NUM_ARGS} ]
then
    echo "ERROR: wrong number of arguments"
    exit 1
fi

writefile=$1
writestr=$2
writedir=$(dirname $writefile)

[ -d $writedir ] || mkdir -p $writedir
echo ${writestr} >> $writefile
