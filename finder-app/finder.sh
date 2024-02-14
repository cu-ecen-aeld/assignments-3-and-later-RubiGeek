#!/bin/sh
expected_args=2
if [ $# -ne ${expected_args} ]
then
	echo "ERROR: Invalid number of arguments."
	echo "Total number of arguments should be 2."
	echo "The order of the arguments should be:"
	echo "	1)File Directory Path"
	echo "	2)String to be searched in the specified directory path."
	exit 1
fi

if [ ! -e "$1" ]
then
	echo "directory $1 does not exist!"
	exit 1
fi

filesdir=$1
searchstr=$2

num_files=$(find ${filesdir} -type f | wc -l)
files_find=$(grep -rn "${searshstr}" ${filesdir} | wc -l)

echo "The number of files are ${num_files} and the number of matching lines are ${files_find}"