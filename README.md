# Introduction

Peer to peer communication platform using Qt/QML and PJSIP library.
Supported plaforms:

- iOS

- Android

- macOS

- Windows

- Linux


# Requirements

- [Qt v5.15](https://www.qt.io)

- [PJSIP library v2.11](https://github.com/pjsip/pjproject/releases)

- [ZeroConf libraries](https://github.com/jbagg/QtZeroConf)

- [Opus library](https://opus-codec.org)


# Clone the Repository

Note that the repository has one submodule, such that the command used to clone the sources is

    git clone --recurse-submodules <repository URL>


# Opus Library Cross-Compilation on macOS

Opus library must be compiled before PJSIP library in order to enable the Opus codec.
Opus library download and compilation is done using build-libopus.sh script. Before running
the script, edit the script and make sure that VERSION and MINIOSVERSION have the correct values.
VERSION must be the latest Opus library version.
MINIOSVERSION must have the same value as in *.pro file, QMAKE_IOS_DEPLOYMENT_TARGET.
Create first a folder right next to the application sources root folder, Opus-iOS, copy
build-libopus.sh script inside the folder and run it. Header files and the Opus library are generated
in Opus-iOS/dependences folder.


# PJSIP Library Cross-Compilation on macOS

- decompress PJSIP sources in a folder next to the application sources root folder

- compile PJSIP library by copying the provided bash script from the tools folder, build-pjsip-ios.sh,
into PJSIP sources root folder, then run the script. The generated libraries and header files are in
build-ios folder, found next to PJSIP sources root folder.


# Application Compilation for macOS

- open with QtCreator intercom.pro in order to compile the application

Note: you might need to install with brew Opus library for macOS.


# Compile and Upload Installer to TestFlight

- use build-ios.sh

- XCode is automatically open on the generated XCode project, then select Product -> Archive

Note: open the script and verify that the paths point to the correct locations (install path of Qt framework)

# Screenshots

![Start Screen](screenshots/start.png?raw=true "Start Screen")

![Service List](screenshots/service_list.png?raw=true "Service List")
