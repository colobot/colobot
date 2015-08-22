#!/bin/bash

if [ -z "$BUILD_DIR" ]; then BUILD_DIR=./build; fi
if [ -z "$COLOBOT_LINT" ]; then COLOBOT_LINT=colobot-lint; fi

diff_files="$(git diff --name-only "$@")"
if [ "$diff_files" == "" ]; then
    exit 0
fi

files=""
root="$(git rev-parse --show-toplevel)"
for file in $diff_files; do
    files="$files $root/$file"
done

output_filters=$(git diff --unified=0 "$@" | \
    awk '
        match($0, /^\+\+\+ b\/(.*)$/, cap) {
            file=cap[1];
        }
        match($0, /^@@.* \+([^,]*),?(.*) @@/, cap) {
            if (cap[2] == "0") {
                // nothing (lines removed)
            } else if (cap[2] == "") {
                // single line
                print "-output-filter " file ":" cap[1] ":" cap[1];
            } else {
                // line range
                print "-output-filter " file ":" cap[1] ":" (cap[1] + cap[2] - 1);
            }
        }
    ')

if [ "$output_filters" == "" ]; then
    exit 0
fi

$COLOBOT_LINT -p $BUILD_DIR $output_filters $files
