#!/bin/bash

# Script to automatically replace patterns in all source files
# Example usage
# (in main directory colobot):
# $ tools/sed-replace.sh src/app/d3dengine.cpp ...
# $ tools/sed-replace.sh `find . -name '*.cpp' -o -name '*.h'`

# List of sed commands (replacements)
replacements=( \
's/\bSetProfileString\b/SetLocalProfileString/g' \
's/\bGetProfileString\b/GetLocalProfileString/g' \
's/\bSetProfileInt\b/SetLocalProfileInt/g' \
's/\bGetProfileInt\b/GetLocalProfileInt/g' \
's/\bSetProfileFloat\b/SetLocalProfileFloat/g' \
's/\bGetProfileFloat\b/GetLocalProfileFloat/g' \
)

# Loop over arguments
find -type f \( -name '*.cpp' -o -name '*.h' \) -print0 | while read -d $'\0' file; do
        echo "Processing file '$file'..."
        # Loop over replacements
        for what in "${replacements[@]}"; do
            sed -i "$what" "$file"
        done
done
