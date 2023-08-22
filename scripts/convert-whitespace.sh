#!/bin/bash

# A script for converting whitespace from old format to new
# For each argument, replaces tabs with 4 spaces and DOS line endings to UNIX

for file in "$@"; do
	dos2unix "$file"
	expand -t 4 "$file" > "$file.out"
	mv -f "$file.out" "$file"
done
