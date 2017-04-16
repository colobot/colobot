# Compiling and installing on Ubuntu Linux

## Install dependencies

    $ apt-get install build-essential cmake libsdl1.2debian libsdl1.2-dev libsdl-image1.2 libsdl-image1.2-dev libsdl-ttf2.0-0 libsdl-ttf2.0-dev libsdl-mixer1.2 libsdl-mixer1.2-dev libsndfile1-dev libvorbis-dev libogg-dev libpng12-dev libglew-dev libopenal-dev libboost-dev libboost-system-dev libboost-filesystem-dev libboost-regex-dev git

## Download sources

    $ git clone https://github.com/colobot/colobot.git /path/to/colobot
    $ cd /path/to/colobot
    $ git submodule update --init

## Compile binaries

    $ cd /path/to/colobot
    $ mkdir build
    $ cd build

Substitute `/installation-directory` in the command below with any directory of your choice (`/tmp/colobot-temporary-install` is ok if you are doing this for the first time):

    $ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/installation-directory ..
    $ make
    $ make install

## Run Colobot

    $ cd /installation-directory
    $ games/colobot
