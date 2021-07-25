#!/bin/bash

APP_NAME=intercom

QT_VER=5.15.2
QT_ROOT=$HOME/Qt/$QT_VER/ios

rm -rf build
mkdir -p build
cd build

$QT_ROOT/bin/qmake .. \
    -spec macx-ios-clang \
    CONFIG+=release \
    CONFIG+=iphoneos \
    CONFIG+=device \
    CONFIG+=qml_debug \
    QMAKE_MAC_XCODE_SETTINGS+=qteam qteam.name=DEVELOPMENT_TEAM qteam.value=FAARUB626Q
make -j

open $APP_NAME.xcodeproj

