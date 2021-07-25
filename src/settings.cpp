#include "settings.h"
#include "config.h"
#include <QSettings>
#include <QVector>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QQmlEngine>

#define INPUT_AUDIO_MODEL_NAME "INPUT AUDIO MODEL NAME"
#define INPUT_AUDIO_MODEL_INDEX "INPUT AUDIO MODEL INDEX"

#define OUTPUT_AUDIO_MODEL_NAME "OUTPUT AUDIO MODEL NAME"
#define OUTPUT_AUDIO_MODEL_INDEX "OUTPUT AUDIO MODEL INDEX"

#define RING_TONES_MODEL_INDEX "RING TONES MODEL INDEX"

#define MICROPHONE_VOLUME "MICROPHONE VOLUME"
#define SPEAKERS_VOLUME "SPEAKERS VOLUME"
#define DIALPAD_VOLUME "DIALPAD VOLUME"

#define BITRATE "BITRATE"
#define SAMPLING_RATE "SAMPLING RATE"
#define FRAME_SIZE "FRAME SIZE"
#define LOST_CONN_TIMEOUT "LOST CONN TIMEOUT"
#define VARIABLE_BITRATE "VARIABLE BITRATE"

#define SETTINGS_FOLDER ".airbytes"
#define REC_FOLDER "AirbytesRecordings"

Settings *_instance = nullptr;

Settings::Settings(QObject *parent) : QObject (parent)
{
    qmlRegisterInterface<Settings>("Settings", 1);
    qmlRegisterType<Settings>("Settings", 1, 0, "Settings");

    _instance = this;
    load();
}

const QString& Settings::writablePath()
{
    static QString path;
    if (path.isEmpty()) {
#if defined (Q_OS_IOS)
        path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/" SETTINGS_FOLDER;
#else
        path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/" SETTINGS_FOLDER;
#endif
        bool rc = QDir(path).exists();
        if (!rc) {
            rc = QDir().mkpath(path);
            if (!rc) {
                qCritical() << "Cannot create" << path;
                path.clear();
            }
        }
    }
    return path;
}

void Settings::clear()
{
    qDebug() << "Clear settings";

    setInputAudioModelIndex(0);
    setOutputAudioModelIndex(0);
    setRingTonesModelIndex(0);

    setMicrophoneVolume(1);
    setSpeakersVolume(1);
    setDialpadSoundVolume(0.5);

    setBitrateKbps(PJMEDIA_CODEC_OPUS_DEFAULT_BIT_RATE / 1000);
    setSampRateKhz(PJMEDIA_CODEC_OPUS_DEFAULT_SAMPLE_RATE / 1000);
    setFrameSizeMs(PJSUA_DEFAULT_AUDIO_FRAME_PTIME);
    setLostConnTimeoutSec(15);
    setIsVariableBitRate(PJ_FALSE == PJMEDIA_CODEC_OPUS_DEFAULT_CBR);

    uninstallClear();
}

void Settings::uninstallClear()
{
    QSettings settings(ORG_NAME, APP_NAME);
    settings.clear();

    const auto path = writablePath();
    if (!path.isEmpty()) {
        QDir(path).removeRecursively();
    }
}

void Settings::load()
{
    QSettings settings(ORG_NAME, APP_NAME);
    qDebug() << "Reading settings from" << settings.fileName();

    //indices for audio devices are loaded and saved separately

    setRingTonesModelIndex(settings.value(RING_TONES_MODEL_INDEX,
                                          _ringTonesModelIndex).toInt());

    setMicrophoneVolume(settings.value(MICROPHONE_VOLUME, _microphoneVolume).toDouble());
    setSpeakersVolume(settings.value(SPEAKERS_VOLUME, _speakersVolume).toDouble());
    setDialpadSoundVolume(settings.value(DIALPAD_VOLUME, _dialpadSoundVolume).toDouble());

    setBitrateKbps(settings.value(BITRATE, _bitrateKbps).toInt());
    setSampRateKhz(settings.value(SAMPLING_RATE, _sampRateKhz).toInt());
    setFrameSizeMs(settings.value(FRAME_SIZE, _frameSizeMs).toDouble());
    setLostConnTimeoutSec(settings.value(LOST_CONN_TIMEOUT, _lostConnTimeoutSec).toInt());
    setIsVariableBitRate(settings.value(VARIABLE_BITRATE, _isVariableBitRate).toBool());
}

void Settings::save()
{
    qDebug() << "Save settings";
    QSettings settings(ORG_NAME, APP_NAME);

    settings.setValue(RING_TONES_MODEL_INDEX, _ringTonesModelIndex);

    settings.setValue(MICROPHONE_VOLUME, _microphoneVolume);
    settings.setValue(SPEAKERS_VOLUME, _speakersVolume);
    settings.setValue(DIALPAD_VOLUME, _dialpadSoundVolume);

    settings.setValue(BITRATE, _bitrateKbps);
    settings.setValue(SAMPLING_RATE, _sampRateKhz);
    settings.setValue(FRAME_SIZE, _frameSizeMs);
    settings.setValue(LOST_CONN_TIMEOUT, _lostConnTimeoutSec);
    settings.setValue(VARIABLE_BITRATE, _isVariableBitRate);
}

AudioDevices::DeviceInfo Settings::inputAudioDeviceInfo()
{
    QSettings settings(ORG_NAME, APP_NAME);
    return { settings.value(INPUT_AUDIO_MODEL_NAME).toString(),
                settings.value(INPUT_AUDIO_MODEL_INDEX,
                               PJMEDIA_AUD_INVALID_DEV).toInt() };
}

void Settings::saveInputAudioDeviceInfo(const AudioDevices::DeviceInfo &devInfo)
{
    QSettings settings(ORG_NAME, APP_NAME);
    settings.setValue(INPUT_AUDIO_MODEL_NAME, devInfo.name);
    settings.setValue(INPUT_AUDIO_MODEL_INDEX, devInfo.index);
}

AudioDevices::DeviceInfo Settings::outputAudioDeviceInfo()
{
    QSettings settings(ORG_NAME, APP_NAME);
    return { settings.value(OUTPUT_AUDIO_MODEL_NAME).toString(),
                settings.value(OUTPUT_AUDIO_MODEL_INDEX,
                               PJMEDIA_AUD_INVALID_DEV).toInt() };
}

void Settings::saveOutputAudioDeviceInfo(const AudioDevices::DeviceInfo &devInfo)
{
    QSettings settings(ORG_NAME, APP_NAME);
    settings.setValue(OUTPUT_AUDIO_MODEL_NAME, devInfo.name);
    settings.setValue(OUTPUT_AUDIO_MODEL_INDEX, devInfo.index);
}
