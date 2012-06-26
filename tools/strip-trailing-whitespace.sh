#!/bin/bash

# A script for removing trailing whitespace from all lines

for file in "$@"; do
	sed -i 's/[ \t]*$//' "$file"
done
