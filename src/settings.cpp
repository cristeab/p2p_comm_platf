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
#define ALWAYS_ON_TOP "ALWAYS ON TOP"
#define LOST_CONN_TIMEOUT "LOST CONN TIMEOUT"

#define SETTINGS_FOLDER ".p2pcomm"

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
    setAlwaysOnTop(true);
    setLostConnTimeoutSec(15);

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
    setAlwaysOnTop(settings.value(ALWAYS_ON_TOP, _alwaysOnTop).toBool());
    setLostConnTimeoutSec(settings.value(LOST_CONN_TIMEOUT, _lostConnTimeoutSec).toInt());
}

void Settings::save()
{
    qDebug() << "Save settings";
    QSettings settings(ORG_NAME, APP_NAME);

    settings.setValue(RING_TONES_MODEL_INDEX, _ringTonesModelIndex);

    settings.setValue(MICROPHONE_VOLUME, _microphoneVolume);
    settings.setValue(SPEAKERS_VOLUME, _speakersVolume);
    settings.setValue(ALWAYS_ON_TOP, _alwaysOnTop);
    settings.setValue(LOST_CONN_TIMEOUT, _lostConnTimeoutSec);
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
