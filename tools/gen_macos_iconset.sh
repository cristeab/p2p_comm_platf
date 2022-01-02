#!/bin/sh
# NB: must be run from the source root folder

ORIGICON=img/logo.png
OUTDIR=macos/icons
FILENAME=AppIcon

# function to generate a single icon
gen_icon () {
    sips -z $1 $1 $ORIGICON --out ${OUTDIR}/${2}.png ;
}

# function to generate a single launch image
gen_launch_image () {
    sips -z $2 $1 $ORIGICON --out ${OUTDIR}/${3}.png ;
}

mkdir -p $OUTDIR

# Icons for iPhone-only Applications
gen_icon 16 "${FILENAME}16x16"
gen_icon 32 "${FILENAME}16x16@2x"
gen_icon 64 "${FILENAME}32x32@2x"
gen_icon 128 "${FILENAME}128x128"
gen_icon 256 "${FILENAME}128x128@2x"
gen_icon 512 "${FILENAME}256x256@2x"
gen_icon 1024 "${FILENAME}512x512@2x"

