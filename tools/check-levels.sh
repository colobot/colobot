#!/bin/bash

# Runs every level in scenetest mode

levels=`ls /usr/local/share/games/colobot/levels | cut -d "." -f 1`
for level in $levels; do
	echo $level
	colobot -runscene $level -scenetest -loglevel warn
done
