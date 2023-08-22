#!/bin/bash

# Formats files to have one newline at the end of file

for file in $@; do
awk '/^$/ { ws++; next; }

{
    for (i = 0; i < ws; ++i) { print ""; }
    print $0;
    ws = 0;
}

END {
    print "";
}' $file > ${file}.out && mv ${file}.out $file
done
