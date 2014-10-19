#!/bin/bash

# Runs every level in scenetest mode

categories=`ls /usr/local/share/games/colobot/levels`
for category in $categories; do
	if [ "$category" = "other" ]; then continue; fi
	chapters=`ls /usr/local/share/games/colobot/levels/$category`
	for chapter in $chapters; do
		chapter=`echo -n $chapter | tail -c 1`
		levels=`ls /usr/local/share/games/colobot/levels/$category/chapter00$chapter`
		for level in $levels; do
			if [ ! -d /usr/local/share/games/colobot/levels/$category/chapter00$chapter/$level ]; then continue; fi
			level=`echo -n $level | cut -d . -f 1 | tail -c 3`
			echo $category$chapter$level
			colobot -runscene $category$chapter$level -scenetest -loglevel warn
		done
	done
done
