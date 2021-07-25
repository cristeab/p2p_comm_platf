#!/bin/bash

CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
TARGET_DIR=$CUR_DIR/../build-ios

make clean && make distclean
rm -rf ../$TARGET_DIR/*

./configure-iphone --prefix $TARGET_DIR --disable-libwebrtc --with-opus=`pwd`/../Opus-iOS/dependencies

cp pjlib/include/pj/config_site_sample.h pjlib/include/pj/config_site.h

# build with minimal size and enable video support
sed -i '' '22i\
#define PJ_CONFIG_IPHONE 1\
#define PJMEDIA_HAS_VIDEO 0\
#define PJ_MAXPATH 520\
#define PJSIP_MAX_PKT_LEN 8000\
#define PJ_LOG_MAX_SIZE (PJSIP_MAX_PKT_LEN+500)\
#define PJSIP_TCP_KEEP_ALIVE_INTERVAL    58\
#define PJSIP_TLS_KEEP_ALIVE_INTERVAL    58\
#define PJMEDIA_RTP_PT_TELEPHONE_EVENTS 101\
#define PJMEDIA_RTP_PT_TELEPHONE_EVENTS_STR "101"\
#define PJMEDIA_RTP_USE_OUR_PT 1
' pjlib/include/pj/config_site.h

make dep && make
make install
