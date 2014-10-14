#!/bin/bash
# Replaces license text with contents of file provided as an argument
if [ ! $# -eq 1 ]; then
	echo "Usage: $0 [file with new license header]"
	exit 1
fi
find . -name "*.cpp" -or -name "*.h" | while read file; do
	echo $file
	sed '/\/\/ \* This.*/,/If not\, see.*/d' $file > $file.tmp
	cat $1 $file.tmp > $file
	rm $file.tmp
done
