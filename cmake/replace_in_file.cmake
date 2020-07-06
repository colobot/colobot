# The script replaces all occurences of REGEX_MATCH with the REPLACE_WITH string.
# The regex rules are like in CMake except the matches are done line by line.
# The purpose of the script is to remove the dependency on `sed` for simple substitutions.
# It's not a function so that it can be used in add_custom_command().
#
# Parameters:
# - REGEX_MATCH
# - REPLACE_WITH
# - FILE_PATH
#
# Example usage:
#    $ cmake -DREGEX_MATCH="^(Text).*$" -DREPLACE_WITH="\1\1\n" -DFILE_PATH="test.txt" -P replace_in_file.cmake
# This will change "Text\n" into "TextText\n" in a file "test.txt".

# ^ and $ work on the whole input
# so we have to split the file by lines in order to have the ^ and $ working like in `sed`
file(STRINGS "${FILE_PATH}" LINES ENCODING UTF-8)

# Workaround for: lists in CMAKE are broken by the `[` and `]` characters
#    Replace them before processing and then place them back
# Caveat: there should not be any placeholder strings in the original file,
#    or they will be changed to brackets too
string(REPLACE "[" "COLOBOT_CMAKE_LEFT_SQUARE_BRACKET" LINES "${LINES}")
string(REPLACE "]" "COLOBOT_CMAKE_RIGHT_SQUARE_BRACKET" LINES "${LINES}")

# Process the file
set(FILE_STRING_OUT "")
foreach(LINE IN LISTS LINES)
    string(REGEX REPLACE "${REGEX_MATCH}" "${REPLACE_WITH}" LINE_OUT "${LINE}")
    string(APPEND FILE_STRING_OUT "${LINE_OUT}" "\n")
endforeach()

# Replace the placeholders with brackets before saving
string(REPLACE "COLOBOT_CMAKE_LEFT_SQUARE_BRACKET" "[" FILE_STRING_OUT "${FILE_STRING_OUT}")
string(REPLACE "COLOBOT_CMAKE_RIGHT_SQUARE_BRACKET" "]" FILE_STRING_OUT "${FILE_STRING_OUT}")

file(WRITE "${FILE_PATH}" "${FILE_STRING_OUT}")
