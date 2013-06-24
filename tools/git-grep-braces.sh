#!/bin/bash

# Finds all "hanging" braces
git grep -n -E '^( *[^ ]+)+[^@]\{\s*$' -- '*.cpp' '*.h'
