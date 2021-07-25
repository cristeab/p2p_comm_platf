#!/bin/sh
# NB: must be run from the source root folder

ORIGICON=img/background.png
OUTDIR=ios/icons
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
gen_icon 40 "${FILENAME}20x20@2x"
gen_icon 60 "${FILENAME}20x20@3x"
gen_icon 58 "${FILENAME}29x29@2x"
gen_icon 87 "${FILENAME}29x29@3x"
gen_icon 80 "${FILENAME}40x40@2x"
gen_icon 120 "${FILENAME}40x40@3x"
gen_icon 180 "${FILENAME}60x60@3x"
gen_icon 20 "${FILENAME}20x20"
gen_icon 29 "${FILENAME}29x29"
gen_icon 76 "${FILENAME}76x76"
gen_icon 152 "${FILENAME}76x76@2x"
gen_icon 167 "${FILENAME}83.5x83.5@2x"
gen_icon 1024 "${FILENAME}1024x1024"

