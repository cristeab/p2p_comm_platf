TEMPLATE = app
QT += core qml quick widgets svg gui multimedia
TARGET = p2pComm
APP_VERSION = 1.0.0

CONFIG += c++17
CONFIG += qtquickcompiler
CONFIG += sdk_no_version_check

#CONFIG(release, debug|release) {
#    DEFINES += QT_NO_DEBUG_OUTPUT
#    DEFINES += QT_NO_INFO_OUTPUT
#    DEFINES += QT_NO_WARNING_OUTPUT
#}

INCLUDEPATH += $$PWD/src \
               $$PWD/3rdparty/QtZeroConf
DESTDIR = build

HEADERS += $$PWD/src/softphone.h \
           $$PWD/src/settings.h \
           $$PWD/src/qmlhelpers.h \
           $$PWD/src/models/active_call_model.h \
           $$PWD/src/models/audio_devices.h \
           $$PWD/src/models/generic_devices.h \
           $$PWD/src/models/ring_tones_model.h \
           $$PWD/3rdparty/QtZeroConf/qzeroconfservice.h \
           $$PWD/3rdparty/QtZeroConf/qzeroconfglobal.h \
           $$PWD/3rdparty/QtZeroConf/qzeroconf.h

SOURCES += $$PWD/src/main.cpp \
           $$PWD/src/softphone.cpp \
           $$PWD/src/settings.cpp \
           $$PWD/src/models/active_call_model.cpp \
           $$PWD/src/models/audio_devices.cpp \
           $$PWD/src/models/ring_tones_model.cpp \
           $$PWD/3rdparty/QtZeroConf/qzeroconfservice.cpp

RESOURCES += res.qrc

#generate configuration file
RESOURCE_CONTENT = "$${LITERAL_HASH}pragma once" \
                   "$${LITERAL_HASH}define APP_VERSION \"$${APP_VERSION}\"" \
                   "$${LITERAL_HASH}define ORG_NAME \"Bogdan Cristea\"" \
                   "$${LITERAL_HASH}define APP_NAME \"p2pComm\""

GENERATED_RESOURCE_FILE = $$OUT_PWD/config.h
write_file($$GENERATED_RESOURCE_FILE, RESOURCE_CONTENT) | error("Aborting.")

mac {
    HEADERS += $$PWD/3rdparty/QtZeroConf/bonjour_p.h
    SOURCES += $$PWD/3rdparty/QtZeroConf/bonjour.cpp
    LIBS += -framework CoreServices
}

ios {
    # Set "Target"
    QMAKE_IOS_DEPLOYMENT_TARGET = 12.0

    # Set "Devices" (1=iPhone, 2=iPad, 1,2=Universal)
    QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 1,2

    QMAKE_INFO_PLIST = ios/Info.plist

    ios_icon.files = $$files($$PWD/ios/icons/AppIcon*.png)
    QMAKE_BUNDLE_DATA += ios_icon

    #launch_images.files = $$PWD/ios/LaunchScreen.xib $$files($$PWD/ios/LaunchImage*.png)
    #QMAKE_BUNDLE_DATA += launch_images

    HEADERS += $$PWD/ios/src/audio_session.h \
               $$PWD/ios/src/app_delegate.h
    SOURCES += $$PWD/ios/src/audio_session.mm \
               $$PWD/ios/src/app_delegate.mm
    INCLUDEPATH += $$PWD/ios/src

    LIBOPUS_BUILD_FOLDER = Opus-iOS/dependencies

    DEFINES += PJ_AUTOCONF
    PJSIP_BUILD_FOLDER = build-ios
    INCLUDEPATH += $$PWD/../$${PJSIP_BUILD_FOLDER}/include

    PJSIP_LIB_SUFFIX = arm64-apple-darwin_ios
    LIBS += -lpjsua2-$${PJSIP_LIB_SUFFIX} \
            -lpjsua-$${PJSIP_LIB_SUFFIX} \
            -lpjsip-ua-$${PJSIP_LIB_SUFFIX} \
            -lpjsip-simple-$${PJSIP_LIB_SUFFIX} \
            -lpjsip-$${PJSIP_LIB_SUFFIX} \
            -lpjmedia-codec-$${PJSIP_LIB_SUFFIX} \
            -lpjmedia-$${PJSIP_LIB_SUFFIX} \
            -lpjmedia-audiodev-$${PJSIP_LIB_SUFFIX} \
            -lpjnath-$${PJSIP_LIB_SUFFIX} \
            -lpjlib-util-$${PJSIP_LIB_SUFFIX} \
            -lsrtp-$${PJSIP_LIB_SUFFIX} \
            -lresample-$${PJSIP_LIB_SUFFIX} \
            -lgsmcodec-$${PJSIP_LIB_SUFFIX} \
            -lspeex-$${PJSIP_LIB_SUFFIX} \
            -lilbccodec-$${PJSIP_LIB_SUFFIX} \
            -lg7221codec-$${PJSIP_LIB_SUFFIX} \
            -lpj-$${PJSIP_LIB_SUFFIX} \
            -lyuv-$${PJSIP_LIB_SUFFIX} \
            -L$$PWD/../$${PJSIP_BUILD_FOLDER}/lib \
            -lopus \
            -L$$PWD/../$${LIBOPUS_BUILD_FOLDER}/lib \
            -framework CFNetwork \
            -framework Contacts \
            -framework BackgroundTasks

    Q_ENABLE_BITCODE.name = ENABLE_BITCODE
    Q_ENABLE_BITCODE.value = NO
    QMAKE_MAC_XCODE_SETTINGS += Q_ENABLE_BITCODE

    Q_XCODE_DEVELOPMENT_TEAM.name = DEVELOPMENT_TEAM
    Q_XCODE_DEVELOPMENT_TEAM.value = FAARUB626Q
    QMAKE_MAC_XCODE_SETTINGS += Q_XCODE_DEVELOPMENT_TEAM

    QMAKE_TARGET_BUNDLE_PREFIX = cristeab.proplife

    OTHER_FILES += ios/Info.plist

    ios_icon.files = $$files($$PWD/ios/icons/AppIcon*.png)
    QMAKE_BUNDLE_DATA += ios_icon
    QMAKE_ASSET_CATALOGS += ios/Images.xcassets

    launch_images.files = $$files($$PWD/img/background.png)
    QMAKE_BUNDLE_DATA += launch_images
    launch_screen.files = $$PWD/ios/CustomLaunchScreen.storyboard
    QMAKE_BUNDLE_DATA += launch_screen

    APP_ENTITLEMENTS.name = CODE_SIGN_ENTITLEMENTS
    APP_ENTITLEMENTS.value = $$PWD/ios/Entitlements.plist
    QMAKE_MAC_XCODE_SETTINGS += APP_ENTITLEMENTS
}

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0
    ICON = img/p2pcomm.icns

    HEADERS += $$PWD/macos/src/dock_click_handler.h
    SOURCES += $$PWD/macos/src/dock_click_handler.mm
    INCLUDEPATH += $$PWD/macos/src

    DEFINES += PJ_AUTOCONF
    PJSIP_BUILD_FOLDER = build-macos
    INCLUDEPATH += $$PWD/../$${PJSIP_BUILD_FOLDER}/include

    PJSIP_LIB_SUFFIX = x86_64-apple-darwin20.5.0
    LIBS += -lpjsua2-$${PJSIP_LIB_SUFFIX} \
            -lpjsua-$${PJSIP_LIB_SUFFIX} \
            -lpjsip-ua-$${PJSIP_LIB_SUFFIX} \
            -lpjsip-simple-$${PJSIP_LIB_SUFFIX} \
            -lpjsip-$${PJSIP_LIB_SUFFIX} \
            -lpjmedia-codec-$${PJSIP_LIB_SUFFIX} \
            -lpjmedia-$${PJSIP_LIB_SUFFIX} \
            -lpjmedia-audiodev-$${PJSIP_LIB_SUFFIX} \
            -lpjnath-$${PJSIP_LIB_SUFFIX} \
            -lpjlib-util-$${PJSIP_LIB_SUFFIX} \
            -lsrtp-$${PJSIP_LIB_SUFFIX} \
            -lresample-$${PJSIP_LIB_SUFFIX} \
            -lgsmcodec-$${PJSIP_LIB_SUFFIX} \
            -lspeex-$${PJSIP_LIB_SUFFIX} \
            -lilbccodec-$${PJSIP_LIB_SUFFIX} \
            -lg7221codec-$${PJSIP_LIB_SUFFIX} \
            -lpj-$${PJSIP_LIB_SUFFIX} \
            -lyuv-$${PJSIP_LIB_SUFFIX} \
            -L$$PWD/../$${PJSIP_BUILD_FOLDER}/lib \
            -framework AudioToolbox \
            -framework CoreAudio \
            /usr/local/Cellar/gnutls/3.6.16_1/lib/libgnutls.30.dylib \
            /usr/local/lib/libopencore-amrnb.a \
            /usr/local/Cellar/opus/1.3.1/lib/libopus.0.dylib
}

OTHER_FILES +=  README.md \
                LICENSE \
                build-ios.sh \
                build-macos.sh \
                tools/build-libopus.sh \
                tools/build-pjsip-ios.sh \
                tools/build-pjsip-macos.sh \
                tools/gen_ios_iconset.sh \
                tools/gen_icns.sh \
                ios/Entitlements.plist
