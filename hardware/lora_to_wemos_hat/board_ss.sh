#!/bin/bash
DIRNAME=`dirname $0`
#echo the name of the folder we're in
echo $DIRNAME
#wait 3 seconds for user to alt-tab to pcbnew
sleep 3
#take a snapshot of the board
gnome-screenshot --file=$DIRNAME/board.png --area
cd $DIRNAME
git add board.png
#view the picture we took to see if we want to commit it or try again
eog board.png
