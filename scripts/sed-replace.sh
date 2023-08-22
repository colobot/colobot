#!/bin/bash

# Script to automatically replace patterns in all source files
# Example usage
# (in main directory colobot):
# $ tools/sed-replace.sh src/app/d3dengine.cpp ...
# $ tools/sed-replace.sh `find . -name '*.cpp' -o -name '*.h'`

# List of sed commands (replacements)
replacements=( \
's/\bD3DVECTOR\b/Math::Vector/g' \
's/\bD3DMATRIX\b/Math::Matrix/g' \
)

# Loop over arguments
for file in "$@"; do
	# Loop over replacements
	for what in "${replacements[@]}"; do
		sed -i "$what" "$file"
	done
	echo "$file"
done
