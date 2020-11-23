#!/bin/sh

# return true if both files are the same
function the_same {
	[ -z "$(diff -q $1 $2)" ]
}

make # build the program

./nflate samples/pandp.txt.gz
if the_same pandp.txt samples/pandp.txt
then
	echo "Succeeded"
else
	echo "Failed"
fi
