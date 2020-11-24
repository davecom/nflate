#!/bin/sh

# return true if both files are the same
function the_same {
	[ -z "$(diff -q $1 $2)" ]
}

make # build the program

files=(samples/pandp.txt.gz samples/house.jpg.gz) 
i=0 

#test each file, one at a time
while [ $i -lt ${#files[@]} ] 
do
	test_file="${files[$i]}"
	decompressed_file="decompressed$i"
	./nflate "$test_file" "$decompressed_file"
	
	if the_same "${test_file%.*}" "$decompressed_file"
	then
		echo "$test_file Test Passed"
	else
		echo "$test_file Test Failed"
	fi
	
	# remove unzipped file
	rm "$decompressed_file"
	
	i=`expr $i + 1` 
done

# delete binary files
make clean