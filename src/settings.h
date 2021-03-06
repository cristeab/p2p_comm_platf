#pragma once

#include "config.h"
#include "qmlhelpers.h"
#include "pjsua.h"
#include "models/audio_devices.h"
#include <QObject>
#include <QString>

class Settings : public QObject {
    Q_OBJECT
    QML_CONSTANT_PROPERTY(QString, orgName, ORG_NAME)
    QML_CONSTANT_PROPERTY(QString, appName, APP_NAME)
    QML_CONSTANT_PROPERTY(QString, appVersion, APP_VERSION)

    QML_WRITABLE_PROPERTY(int, inputAudioModelIndex, setInputAudioModelIndex, -1)
    QML_WRITABLE_PROPERTY(int, outputAudioModelIndex, setOutputAudioModelIndex, -1)
    //there is a default ring tone
    QML_WRITABLE_PROPERTY(int, ringTonesModelIndex, setRingTonesModelIndex, 0)

    QML_WRITABLE_PROPERTY_FLOAT(qreal, microphoneVolume, setMicrophoneVolume, 2.0)
    QML_WRITABLE_PROPERTY_FLOAT(qreal, speakersVolume, setSpeakersVolume, 2.0)
    QML_WRITABLE_PROPERTY(bool, alwaysOnTop, setAlwaysOnTop, true)
    QML_WRITABLE_PROPERTY(int, lostConnTimeoutSec, setLostConnTimeoutSec, 15)

public:
    enum CodecBitRate { CODEC_OPUS_DEFAULT_BIT_RATE_KBPS = 0,
                        CODEC_OPUS_MIN_BIT_RATE_KBPS = 6,
                        CODEC_OPUS_MAX_BIT_RATE_KBPS = 510 };
    Q_ENUM(CodecBitRate)
    enum CodecSampleRate { CODEC_OPUS_MIN_SAMPLE_RATE_KHZ = 8,
                           CODEC_OPUS_SAMPLE_RATE_0_KHZ = 12,
                           CODEC_OPUS_SAMPLE_RATE_1_KHZ = 16,
                           CODEC_OPUS_SAMPLE_RATE_2_KHZ = 24,
                           CODEC_OPUS_MAX_SAMPLE_RATE_KHZ = 48 };
    Q_ENUM(CodecSampleRate)

    explicit Settings(QObject *parent = nullptr);

    static const QString& writablePath();

    static AudioDevices::DeviceInfo inputAudioDeviceInfo();
    static void saveInputAudioDeviceInfo(const AudioDevices::DeviceInfo &devInfo);

    static AudioDevices::DeviceInfo outputAudioDeviceInfo();
    static void saveOutputAudioDeviceInfo(const AudioDevices::DeviceInfo &devInfo);

    void save();
    void clear();
    static void uninstallClear();
private:
    Q_DISABLE_COPY_MOVE(Settings)
    void load();
};
