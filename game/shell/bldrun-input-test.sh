# Name:    bldrun-input-test.sh
# Purpose: Build and run a test for a Joystick class to interface with a PS3 controller

MAIN_DIR=/home/pi/matrix/game
BIN_DIR=$MAIN_DIR/bin
BIN_NAME=input-test

cd $MAIN_DIR
if make $BIN_NAME; then
        echo
        echo "PROGRAM OUTPUT:"
        sudo $BIN_DIR/$BIN_NAME
fi
