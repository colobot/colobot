#!/bin/bash
# Returns mission time on stdout for a mission given on the commandline
# Make sure the level actually uses MissionTimer, or the script will hang!

colobot -headless -runscene $@ 2>&1 | while read -r line; do
	if [[ $line =~ Mission[[:space:]]time:[[:space:]]([0-9:.]*) ]]; then
		echo ${BASH_REMATCH[1]}
	fi
done
