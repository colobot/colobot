#!/bin/bash

# A script for converting whitespace from old format to new
# For each argument, replaces tabs with 4 spaces and DOS line endings to UNIX

find -type f \( -name '*.cpp' -o -name '*.h' \) -print0 | while read -d $'\0' file
do
    dos2unix "$file"
    expand -t 4 "$file" > "$file.out"
    mv -f "$file.out" "$file"
done
