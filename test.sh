#!/usr/bin/env bash

if [[ $# -lt 3 ]]; then
	echo "Invalid argument count!"
	exit 1
fi

./$1 $2
prog=$?
./$3

if [[ $prog == $? ]]; then
	echo "Test successful!"
else
	echo "Test failed! Prog output was ${prog}, valid output was ${?}"
fi

