#!/bin/bash

# A script for removing trailing whitespace from all lines

find -type f \( -name '*.cpp' -o -name '*.h' \) -print0 | while read -d $'\0' file
do
    sed -i 's/[ \t]*$//' "$file"
done
