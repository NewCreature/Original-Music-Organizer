#!/bin/bash

# Script to launch audio servers for music-making.

case $1 in

  start )
    echo Starting fluidsynth...

    # Start fluidsynth
    fluidsynth --server --no-shell --audio-driver=pulseaudio --midi-driver=alsa_seq \
        --reverb=0 --chorus=0 --gain=0.8 \
        /usr/share/sounds/sf2/FluidR3_GM.sf2 \
        &>/dev/null &

    sleep 1

    if pgrep -l fluidsynth
    then
      echo Audio servers running.
    else
      echo There was a problem starting the audio servers.
    fi

    ;;

  stop )
    killall fluidsynth
    echo Audio servers stopped.
    ;;

  * )
    echo Please specify start or stop...
    ;;
esac
