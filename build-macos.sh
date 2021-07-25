#!/bin/bash

APP_NAME=intercom

QT_VER=5.15.2
QT_ROOT=$HOME/Qt/$QT_VER/clang_64

rm -rf build
mkdir -p build
cd build

$QT_ROOT/bin/qmake .. \
    -spec macx-clang \
    CONFIG+=release \
    CONFIG+=x86_64
make -j

$QT_ROOT/bin/macdeployqt $APP_NAME.app \
                        -qmldir=../qml \
                        -always-overwrite \
                        -dmg \
                        -appstore-compliant
