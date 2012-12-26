#!/bin/sh

set -e

# Create colobot.desktop from various colobot.ini's

fname=colobot.ini

cat colobot.desktop.in

linguas=$([ ! -d lang ] || ( cd lang ; ls));

for type in Name GenericName Comment; do
	egrep "^$type=" $fname | sed -e "s/^$type=\"\(.*\)\"$/$type=\1/g"
	for l in $linguas; do
		egrep "^$type=" lang/$l/$fname | sed -e "s/^$type=\"\(.*\)\"$/$type[$l]=\1/g"
	done
done
