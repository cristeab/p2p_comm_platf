#pragma once

#include "qmlhelpers.h"
#include "settings.h"
#include "models/audio_devices.h"
#include "models/ring_tones_model.h"
#include "models/active_call_model.h"
#include "pj/config_site.h"
#include "pjsua.h"
#include "qzeroconf.h"
#include <QObject>
#include <QTimer>
#include <QString>
#include <QMap>
#include <QElapsedTimer>

class Softphone : public QObject {
	Q_OBJECT

    QML_READABLE_PROPERTY(bool, showBusy, setShowBusy, false)

    QML_WRITABLE_PROPERTY(QString, dialedText, setDialedText, "")
    QML_WRITABLE_PROPERTY(QString, currentUserId, setCurrentUserId, "")
    QML_WRITABLE_PROPERTY(bool, activeCall, setActiveCall, false)

    QML_WRITABLE_PROPERTY(bool, muteMicrophone, setMuteMicrophone, false)
    QML_WRITABLE_PROPERTY(bool, muteSpeakers, setMuteSpeakers, false)

    QML_WRITABLE_PROPERTY(int, winPosX, setWinPosX, 0)
    QML_WRITABLE_PROPERTY(int, winPosY, setWinPosY, 0)
    QML_WRITABLE_PROPERTY(int, winWidth, setWinWidth, 0)

    QML_CONSTANT_PROPERTY_PTR(Settings, settings)
    QML_CONSTANT_PROPERTY_PTR(AudioDevices, inputAudioDevices)
    QML_CONSTANT_PROPERTY_PTR(AudioDevices, outputAudioDevices)
    QML_CONSTANT_PROPERTY_PTR(RingTonesModel, ringTonesModel)
    QML_CONSTANT_PROPERTY_PTR(ActiveCallModel, activeCallModel)

    QML_CONSTANT_PROPERTY(int, invalidCallId, PJSUA_INVALID_ID)

    QML_WRITABLE_PROPERTY(bool, holdCall, setHoldCall, false)
    QML_WRITABLE_PROPERTY(bool, conference, setConference, false)

    QML_READABLE_PROPERTY(double, txLevel, setTxLevel, 0)
    QML_READABLE_PROPERTY(double, rxLevel, setRxLevel, 0)

    QML_READABLE_PROPERTY(QStringList, deviceList, setDeviceList, QStringList())
    QML_READABLE_PROPERTY(QString, currentDeviceName, setCurrentDeviceName, "")

private:
    struct ZeroConfItem {
        QString name;
        QString address;
        QString port;
        QString uuid;
        void clear() {
            name = address = port = uuid = "";
        }
    };

public:
    enum CallState { NONE, OUTGOING, INCOMING, ACTIVE };
    Q_ENUM(CallState)

    Softphone();
    ~Softphone();

    static Softphone* instance();

    bool setAudioDevices();
    Q_INVOKABLE bool init();
    Q_INVOKABLE bool makeCall(int zeroConfIndex);
    Q_INVOKABLE bool answer(int callId);
    Q_INVOKABLE bool hangup(int callId);
    Q_INVOKABLE void hangupAll();
    Q_INVOKABLE bool swap(int callId);
    Q_INVOKABLE bool merge(int callId);
    Q_INVOKABLE void onCurrentUserTimeout();
    Q_INVOKABLE void zeroConf(bool start);
    Q_INVOKABLE bool setupAudioCodecParam();

    bool hold(bool value, int callId);
    bool muteMicrophone(bool value, int callId);
    bool muteSpeakers(bool value, int callId);
    QString convertNumber(const QString &num);
    void release();

    static void showMessage(const QString &msg, bool error);

    static pj_bool_t onRxRequest(pjsip_rx_data *rdata);

signals:
    void disconnected(int callId);
    void confirmed(int callId);
    void calling(int callId, const QString &deviceUuid, const QString &name);
    void incoming(int callCount, int callId, const ZeroConfItem &zcItem, bool isConf);
    void incomingMessageDialog(int callCount, int callId, const QString &userName, bool isConf);
    void phoneStateChanged();
    void audioDevicesChanged();
    void showMessageDialog(const QString &message, bool error, bool retry);

private:
    Q_DISABLE_COPY_MOVE(Softphone)

    enum PhoneState { UNKNOWN = 0x00, PARKING_CALL = 0x01, RECORDING_CALL = 0x02,
                            MERGE_CALLS = 0x04 };
    enum SipErrorCodes { BadRequest = 400, RequestTimeout = 408, RequestTerminated = 487,
                         ServiceUnavailable = 503 };
    enum { DEFAULT_SIP_PORT = 5060, ZERO_CONF_PORT = 11437, RETRY_TIMEOUT_MS = 1000,
           CALL_DURATION_PERIOD_MS = 1000, MAX_CODEC_COUNT = 32,
           ERROR_MSG_SIZE = 1024, STARTUP_TIMEOUT_MS = 3000 };

    static const char *userAgentValue();
    void initAudioDevicesList();

    static void onIncomingCall(pjsua_acc_id acc_id, pjsua_call_id call_id,
                               pjsip_rx_data *rdata);
    static void onCallState(pjsua_call_id call_id, pjsip_event *e);
    static void onCallMediaState(pjsua_call_id call_id);
    static void onStreamCreated(pjsua_call_id call_id, pjmedia_stream *strm,
                                unsigned stream_idx, pjmedia_port **p_port);
    static void onStreamDestroyed(pjsua_call_id call_id, pjmedia_stream *strm,
                                  unsigned stream_idx);
    static void pjsuaLogCallback(int level, const char *data, int len);

    void onConfirmed(int callId);
    void onCalling(int callId, const QString &deviceUuid, const QString &name);
    void onIncoming(int callCount, int callId, const ZeroConfItem &zcItem, bool isConf);
    void onDisconnected(int callId);

    bool initRingTonePlayer(pjsua_call_id id);
    bool startPlayingRingTone(pjsua_call_id id);
    void stopPlayingRingTone(pjsua_call_id id);
    bool releaseRingTonePlayer(pjsua_call_id id);
    void releaseRingTonePlayers();

    static void dumpStreamStats(pjmedia_stream *strm);
    void errorHandler(const QString &title, pj_status_t status = PJ_SUCCESS,
                             bool emitSignal = false);
    bool setMicrophoneVolume(pjsua_conf_port_id portId, bool mute = false);
    bool setSpeakersVolume(pjsua_conf_port_id portId, bool mute = false);
    bool callUri(pj_str_t *uri, int zeroConfIndex, std::string &uriBuffer);

    void connectCallToSoundDevices(pjsua_conf_port_id confPortId);
    static void disconnectCallFromSoundDevices(pjsua_conf_port_id confPortId);

    void setupConferenceCall(pjsua_call_id callId);
    bool setAudioCodecPriority(const QString &codecId, int priority);

    void onMicrophoneVolumeChanged();
    void onSpeakersVolumeChanged();

    static QString toString(const pj_str_t &pjStr) {
        return QString::fromLocal8Bit(pjStr.ptr, static_cast<int>(pjStr.slen));
    }
    static bool isMediaActive(const pjsua_call_media_info &media) {
        return (PJMEDIA_TYPE_AUDIO == media.type) &&
                (PJSUA_CALL_MEDIA_NONE != media.status) &&
                (PJSUA_CALL_MEDIA_ERROR != media.status);
    }
    QString currentTransport() const {
        return _isUdpTransport ? "" : ";transport=tcp";
    }
    static QString serverPort(const QString &port = "") {
        return port.isEmpty() ? "" : (":" + port);
    }
    static void checkMediaType(const pjsua_call_info &callInfo);

    static bool initRegistrar();
    static void simpleRegistrar(pjsip_rx_data *rdata);

    void startPublish();
    void addService(QZeroConfService item);
    void removeService(QZeroConfService item);
    void removeServiceWithUuid(const QString &uuid);
    void updateService(QZeroConfService item);
    void updateServiceWithItem(const ZeroConfItem &zcItem);
    void extractAddressUserName(ZeroConfItem &zcItem, const QString &remoteContact,
                                const QString &localInfo);
    void setupAudioCodecPriority();
    static pjmedia_codec_mgr* pjMediaCodecMgr();

    bool isValidZeroConfIndex(int index) const {
        return (0 <= index) && (index < _zeroConfList.size());
    }

    void onRetryConnection();
    void resetElapsedTimer();
    static QString deviceName();
    static QString generateDeviceUuid();
    void updateCurrentDeviceName(const QString &name, bool add);

    QTimer _currentUserTimer;
    static const QString _notAvailable;
    bool _pjsuaStarted = false;
    QString _localPort;
    pjsua_acc_id _accId = PJSUA_INVALID_ID;
    QHash<pjsua_call_id, pjsua_player_id> _playerId;
    QHash<pjsua_call_id, pjsua_recorder_id> _recId;
    uint8_t _phoneState = PhoneState::UNKNOWN;
    bool _manualHangup = false;
    bool _isUdpTransport = false;
    QHash<QString,pjsua_buddy_id> _userId2BuddyId;
    QZeroConf _zeroConf;
    QList<ZeroConfItem> _zeroConfList;
    QElapsedTimer _retryElapsedTimer;
    pjmedia_codec_info _opusCodecInfo;
    const QString _deviceUuid;
};
