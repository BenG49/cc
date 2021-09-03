#!/usr/bin/env bash

if [[ $# -lt 2 ]]; then
	echo "Invalid argument count!"
	exit 1
fi

# arg 2 is test file
gcc $2 -o test.out

# arg 1 is executable
program_output=`./$1 2>/dev/null`
program_exit_code=$?

test_output=`./test.out 2>/dev/null`
test_exit_code=$?

rm test.out

if [[ $program_output -ne $test_output ]]; then
	echo "Test failed - program output"
elif [[ $program_exit_code -ne $test_exit_code ]]; then
	echo "Test failed - program exit code"
else
	echo "Test successful"
fi

echo "Program output: ${program_output} - Test output: ${test_output}"
echo "Program exit: ${program_exit_code} - Test exit: ${test_exit_code}"
