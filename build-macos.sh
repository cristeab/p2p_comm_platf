#!/bin/bash

APP_NAME=p2pcommlite

QT_VER=6.2.2
QT_ROOT=$HOME/Qt/$QT_VER/macos

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

if [ -z "$APP_PWD" ]; then
    echo Upload password is not set
    exit 0
fi

echo "Validate ..."
cd ..
xcrun altool --validate-app \
             --file ./build/$APP_NAME.app \
             --type macos \
             --username cristeab@gmail.com \
             --password $APP_PWD
if [ $? -ne 0 ]; then
    exit $?
fi
echo "Upload ..."
xcrun altool --upload-app \
             --file ./build/$APP_NAME.app \
             --type macos \
             --username cristeab@gmail.com \
             --password $APP_PWD
