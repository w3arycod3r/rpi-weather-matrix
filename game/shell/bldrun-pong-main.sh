# Name:    bldrun-pong-main.sh
# Purpose: Build and run Pong! :D

MAIN_DIR=/home/pi/matrix/game
BIN_DIR=$MAIN_DIR/bin
BIN_NAME=pong-main

cd $MAIN_DIR
if make $BIN_NAME; then
        echo
        echo "PROGRAM OUTPUT:"
        sudo $BIN_DIR/$BIN_NAME
fi
