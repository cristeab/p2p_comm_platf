#!/bin/bash

APP_NAME=p2pcommlite

QT_VER=6.2.2
QT_ROOT=$HOME/Qt/$QT_VER/macos

rm -rf build
mkdir -p build
cd build

$QT_ROOT/bin/qmake .. \
    -spec macx-xcode \
    CONFIG+=release \
    CONFIG+=x86_64
open $APP_NAME.xcodeproj
