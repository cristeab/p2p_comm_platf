#include "softphone.h"
#ifdef Q_OS_IOS
#include "audio_session.h"
#include "app_delegate.h"
#endif
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QThread>
#include <QStyle>
#include <QTcpServer>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QUuid>
#include <QCryptographicHash>

#define ZERO_CONF_SIP_PORT "sipPort"
#define ZERO_CONF_DEVICE_UUID "deviceUuid"
#define ZERO_CONF_TYPE "_sip._tcp"

static Softphone *_instance = nullptr;
const QString Softphone::_notAvailable = tr("Unknown");

Softphone::Softphone() : _deviceUuid(generateDeviceUuid())
{
    setObjectName("softphone");
    qmlRegisterType<Softphone>("Softphone", 1, 0, "Softphone");
    qRegisterMetaType<ZeroConfItem>("ZeroConfItem");

    _instance = this;

    //setup call duration timer
    _currentUserTimer.setInterval(CALL_DURATION_PERIOD_MS);
    _currentUserTimer.setSingleShot(false);
    _currentUserTimer.setTimerType(Qt::PreciseTimer);
    _currentUserTimer.stop();
    connect(&_currentUserTimer, &QTimer::timeout, this, &Softphone::onCurrentUserTimeout);

    //init internal connections
    connect(this, &Softphone::confirmed, this, &Softphone::onConfirmed);
    connect(this, &Softphone::calling, this, &Softphone::onCalling);
    connect(this, &Softphone::incoming, this, &Softphone::onIncoming);
    connect(this, &Softphone::disconnected, this, &Softphone::onDisconnected);

    //init connections with active calls model
    connect(_activeCallModel, &ActiveCallModel::activeCallChanged, this, [this](bool value) {
        if (_conference) {
            setDialedText("");
            setTxLevel(0);
            setRxLevel(0);
        } else {
            setActiveCall(value);
            //setDialedText(_activeCallModel->currentPhoneNumber());
        }
    });
    connect(_activeCallModel, &ActiveCallModel::unholdCall, this, [this](int callId) {
        hold(false, callId);
    });

    //init connection with settings
    _inputAudioDevices->setSettings(_settings);
    _outputAudioDevices->setSettings(_settings);
    connect(_settings, &Settings::inputAudioModelIndexChanged, this, [&]() {
        const auto &devInfo = _inputAudioDevices->deviceInfoFromIndex(_settings->inputAudioModelIndex());
        Settings::saveInputAudioDeviceInfo(devInfo);
    });
    connect(_settings, &Settings::outputAudioModelIndexChanged, this, [&]() {
        const auto &devInfo = _outputAudioDevices->deviceInfoFromIndex(_settings->outputAudioModelIndex());
        Settings::saveOutputAudioDeviceInfo(devInfo);
    });

    //connection with mute microphone
    connect(this, &Softphone::muteMicrophoneChanged, _activeCallModel, [this]() {
        const auto &cids = _activeCallModel->confirmedCallsId();
        for (auto id: cids) {
            muteMicrophone(_muteMicrophone, id);
        }
    });
    //connection with enable speakers
    connect(this, &Softphone::muteSpeakersChanged, this, [this]() {
#if defined(Q_OS_IOS)
        audioOutputSpeaker(!_muteSpeakers);
#else
        const auto &cids = _activeCallModel->confirmedCallsId();
        for (auto id: cids) {
            muteSpeakers(_muteSpeakers, id);
        }
#endif
    });
    //connection with hold call
    connect(this, &Softphone::holdCallChanged, this, [this]() {
        const auto cid = _activeCallModel->currentCallId();
        hold(_holdCall, cid);
    });

    //connection with volume settings
    connect(_settings, &Settings::microphoneVolumeChanged, this,
            &Softphone::onMicrophoneVolumeChanged);
    connect(_settings, &Settings::speakersVolumeChanged, this,
            &Softphone::onSpeakersVolumeChanged);

    //ringtones init
    _ringTonesModel->initDefaultRingTones();

    //Zero Conf init
    connect(&_zeroConf, &QZeroConf::servicePublished, this, []() {
        qInfo() << "Service published";
    });
    connect(&_zeroConf, &QZeroConf::error, this, [this](QZeroConf::error_t error) {
        QString msg = tr("ZeroConf error: ");
        switch (error) {
        case QZeroConf::noError:
            msg += tr("No error");
            break;
        case QZeroConf::serviceRegistrationFailed:
            msg += tr("Service registration failed");
            break;
        case QZeroConf::serviceNameCollision:
            msg += tr("Service name collision");
            break;
        case QZeroConf::browserFailed:
            msg += tr("Browser failed");
            break;
        default:
            msg += tr("Unknown");
        }
        qWarning() << msg;
        if (QZeroConf::noError != error) {
            emit showMessageDialog(msg, true, false);
        }
    });
    connect(&_zeroConf, &QZeroConf::serviceAdded, this, &Softphone::addService);
    connect(&_zeroConf, &QZeroConf::serviceRemoved, this, &Softphone::removeService);
    connect(&_zeroConf, &QZeroConf::serviceUpdated, this, &Softphone::updateService);
    connect(this, &Softphone::audioDevicesChanged, this, &Softphone::initAudioDevicesList);

    QTimer::singleShot(STARTUP_TIMEOUT_MS, this, [this]() {
        qInfo() << "Startup";
        if (init()) {
            zeroConf(true);
        }
    });
}

Softphone::~Softphone()
{
    release();
}

Softphone* Softphone::instance()
{
    return _instance;
}

void Softphone::onConfirmed(int callId)
{
    stopPlayingRingTone(callId);

    _activeCallModel->setCallState(callId, ActiveCallModel::CallState::CONFIRMED);
    _activeCallModel->setCurrentCallId(callId);
    if (_retryElapsedTimer.isValid()) {
        resetElapsedTimer();
    }
}

void Softphone::onCalling(int callId, const QString &deviceUuid, const QString &name)
{
    _activeCallModel->addCall(callId, deviceUuid, name);
}

void Softphone::onIncoming(int callCount, int callId, const ZeroConfItem &zcItem, bool isConf)
{
    emit trayIconMessage(tr("Incoming call from ") + zcItem.name);

    emit incomingMessageDialog(callCount, callId, zcItem.name, isConf);

    qDebug() << "Call count" << callCount << ", CID" << callId << ", name" << zcItem.name;
    Q_UNUSED(callCount)
    Q_UNUSED(isConf)
    //open audio device only when needed (automatically closed when the call ends)
    setAudioDevices();
    updateCurrentDeviceName(zcItem.name, true);
    _activeCallModel->addCall(callId, zcItem.uuid, zcItem.name);

    raiseWindow();

    startPlayingRingTone(callId);
    updateServiceWithItem(zcItem);
}

void Softphone::onDisconnected(int callId)
{
    qDebug() << "onDisconnected" << callId;
    stopPlayingRingTone(callId);

    const auto name = _activeCallModel->name(callId);
    updateCurrentDeviceName(name, false);
    _activeCallModel->removeCall(callId);

    if (0 == _activeCallModel->callCount()) {
        setActiveCall(false);
    }
#if defined(Q_OS_IOS)
    disableAudioSession();
    pjsua_set_no_snd_dev();
#endif
}

bool Softphone::setAudioCodecPriority(const QString &codecId, int priority)
{
    if ((0 > priority) || (priority > 255)) {
        qWarning() << "Invalid codec priority" << codecId << priority;
        return false;
    }
    const auto stdCodecId = codecId.toStdString();
    pj_str_t pjCodecId;
    pj_cstr(&pjCodecId, stdCodecId.c_str());
    const auto status = pjsua_codec_set_priority(&pjCodecId, priority);
    if (PJ_SUCCESS == status) {
        //qInfo() << "Changed codec priority" << codecId << priority;
    } else {
        errorHandler("Cannot set codec priority", status);
    }
    return PJ_SUCCESS == status;
}

void Softphone::onIncomingCall(pjsua_acc_id acc_id, pjsua_call_id callId,
                               pjsip_rx_data *rdata)
{
    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);
    if (nullptr == _instance) {
        qCritical() << "Invalid instance";
        return;
    }

    pjsua_call_info ci;
    pjsua_call_get_info(callId, &ci);
    const auto remoteContact = toString(ci.remote_contact);
    const auto localInfo = toString(ci.local_info);
    qInfo() << "Incoming call from" << remoteContact << "to" << localInfo;

    ZeroConfItem zcItem;
    _instance->extractAddressUserName(zcItem, remoteContact, localInfo);

    const auto isConf = _instance->_activeCallModel->isConference();
    emit _instance->incoming(pjsua_call_get_count(), callId, zcItem, isConf);
#ifdef Q_OS_IOS
    //startLocalNotification(userName);
#endif
}

void Softphone::onCallState(pjsua_call_id callId, pjsip_event *e)
{
    PJ_UNUSED_ARG(e);
    if (nullptr == _instance) {
        return;
    }

    pjsua_call_info ci;
    const auto pjStatus = pjsua_call_get_info(callId, &ci);
    if ((PJ_SUCCESS != pjStatus) || (PJSIP_INV_STATE_NULL == ci.state)) {
        qDebug() << "onCallState: nothing to do";
        return;
    }
    const QString info = toString(ci.state_text);
    const QString status = toString(ci.last_status_text);
    qDebug() << "Call" << callId << ", state =" << info << "(" << ci.last_status << ")" << status;

    const auto lastStatus = static_cast<SipErrorCodes>(ci.last_status);
    if (SipErrorCodes::ServiceUnavailable == lastStatus) {
        _instance->removeServiceWithUuid(_instance->_activeCallModel->deviceUuid(callId));
    }

    if ((SipErrorCodes::BadRequest <= lastStatus) &&
            (SipErrorCodes::RequestTerminated != lastStatus) &&
            (SipErrorCodes::RequestTimeout != lastStatus) &&
            !_instance->_manualHangup) {
        //show all SIP errors above Client Failure Responses
        const auto msg = status + " (" + QString::number(ci.last_status) + ")";
        emit _instance->showMessageDialog(msg, true, false);
        if (_instance->activeCallModel()->isConfirmedCall(callId)) {
            if (!_instance->_retryElapsedTimer.isValid()) {
                _instance->_retryElapsedTimer.start();
            }
            QTimer::singleShot(RETRY_TIMEOUT_MS, _instance, &Softphone::onRetryConnection);
            _instance->setShowBusy(true);
            qWarning() << "Retry to connect after" << RETRY_TIMEOUT_MS << "ms";
        }
    }
    _instance->_manualHangup = false;

    switch (ci.state) {
    case PJSIP_INV_STATE_NULL:
        break;
    case PJSIP_INV_STATE_CALLING: {
        ZeroConfItem zcItem;
        const auto remoteContact = toString(ci.remote_contact);
        const auto localInfo = toString(ci.local_info);
        _instance->extractAddressUserName(zcItem, remoteContact, localInfo);
        emit _instance->calling(callId, zcItem.uuid, zcItem.name);
    }
        //outgoing calls use standard ring tone
        break;
    case PJSIP_INV_STATE_INCOMING:
        break;
    case PJSIP_INV_STATE_EARLY:
        break;
    case PJSIP_INV_STATE_CONNECTING:
        break;
    case PJSIP_INV_STATE_CONFIRMED:
        //sound devs could be connected here
        emit _instance->confirmed(callId);//must be last
        Softphone::checkMediaType(ci);
        break;
    case PJSIP_INV_STATE_DISCONNECTED:
        emit _instance->disconnected(callId);
        break;
    default:
        qCritical() << "unhandled call state" << ci.state;
    }
}

void Softphone::onCallMediaState(pjsua_call_id callId)
{
    if (nullptr == _instance) {
        return;
    }
    pjsua_call_info ci;
    pj_status_t status = pjsua_call_get_info(callId, &ci);
    if (PJ_SUCCESS != status) {
        _instance->errorHandler("Cannot get media status", status, false);
        return;
    }
    if (PJSIP_INV_STATE_NULL == ci.state) {
        qDebug() << "onCallMediaState: nothing to do";
        return;
    }
    pj_str_t statusTxt = ci.last_status_text;
    qDebug() << "Media state changed for call" << callId << ":" << toString(statusTxt)
             << ci.last_status;
    if ((PJSUA_CALL_MEDIA_ACTIVE == ci.media_status) ||
            (PJSUA_CALL_MEDIA_REMOTE_HOLD == ci.media_status)) {
        qInfo() << "Media active/remote hold" << ci.media_cnt;
        _instance->connectCallToSoundDevices(ci.conf_slot);
        for (unsigned medIdx = 0; medIdx < ci.media_cnt; ++medIdx) {
            if (isMediaActive(ci.media[medIdx])) {
                pjsua_stream_info streamInfo;
                status = pjsua_call_get_stream_info(callId, medIdx, &streamInfo);
                if (PJ_SUCCESS == status) {
                    if (PJMEDIA_TYPE_AUDIO == streamInfo.type) {
                        const auto &fmt = streamInfo.info.aud.fmt;
                        qInfo() << "Audio codec info: encoding" << toString(fmt.encoding_name)
                                << ", clock rate" << fmt.clock_rate << "Hz, channel count"
                                << fmt.channel_cnt;
                    }
                } else {
                    _instance->errorHandler("Cannot get stream info", status, false);
                }
            }
        }
    } else {
        Softphone::disconnectCallFromSoundDevices(ci.conf_slot);
    }
}

void Softphone::onStreamCreated(pjsua_call_id call_id, pjmedia_stream *strm,
                                unsigned stream_idx, pjmedia_port **p_port)
{
    Q_UNUSED(strm)
    Q_UNUSED(p_port)
    qInfo() << "onStreamCreated" << call_id << stream_idx;
    //dumpStreamStats(strm);
}

void Softphone::onStreamDestroyed(pjsua_call_id call_id, pjmedia_stream *strm,
                                  unsigned stream_idx)
{
    qInfo() << "onStreamDestroyed" << call_id << stream_idx;
    dumpStreamStats(strm);
}

void Softphone::dumpStreamStats(pjmedia_stream *strm)
{
    pjmedia_rtcp_stat stat;
    const pj_status_t status = pjmedia_stream_get_stat(strm, &stat);
    if (PJ_SUCCESS != status) {
        _instance->errorHandler("Cannot get stream stats", status, false);
        return;
    }

    auto showStreamStat = [](const char *prefix, const pjmedia_rtcp_stream_stat &s) {
        qInfo() << prefix << "packets =" << s.pkt << "packets, payload ="
                << s.bytes << "bytes, loss =" << s.loss << "packets, percent loss ="
                << s.loss * 100.0 / (s.loss + s.pkt) << "%, dup =" << s.dup
                << "packets, reorder =" << s.reorder << "packets, discard ="
                << s.discard << "packets, jitter (min, mean, max) ="
                << s.jitter.min << s.jitter.mean << s.jitter.max << "ms";
    };
    showStreamStat("RX stat:", stat.rx);
    showStreamStat("TX stat:", stat.tx);
    qInfo() << "RTT (min, mean, max) =" << stat.rtt.min << stat.rtt.mean
            << stat.rtt.max << "ms";
}

void Softphone::pjsuaLogCallback(int level, const char *data, int /*len*/)
{
    qDebug() << "PJSUA:" << level << data;
}

const char* Softphone::userAgentValue()
{
    //user agent contains app version and PJSIP version
    static const std::string pjsipVer(pj_get_version());
    static const std::string userAgent = (QString(APP_NAME).simplified().replace(" ", "") + "/" +
                             QString(APP_VERSION)).toStdString() +
                             "(PJSIP/" + pjsipVer + ")";
    return userAgent.c_str();
}

bool Softphone::init()
{
    qDebug() << "Init PJSUA library";
    //destroy previous instance if any
    if (_pjsuaStarted) {
        qDebug() << "PJSUA already initialized";
        return true;
    }

    //create PJSUA
    pj_status_t status = pjsua_create();
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot create PJSUA", status, true);
        return false;
    }

    //init PJSUA
    {
        pjsua_config cfg;
        pjsua_logging_config log_cfg;

        pjsua_config_default(&cfg);
        cfg.max_calls = PJSUA_MAX_CALLS;
        cfg.cb.on_incoming_call = &onIncomingCall;
        cfg.cb.on_call_media_state = &onCallMediaState;
        cfg.cb.on_call_state = &onCallState;
        cfg.cb.on_stream_created = &onStreamCreated;
        cfg.cb.on_stream_destroyed = &onStreamDestroyed;

        pj_cstr(&cfg.user_agent, userAgentValue());
        qDebug() << "User agent" << userAgentValue();

        pjsua_logging_config_default(&log_cfg);
        log_cfg.msg_logging = PJ_TRUE;
        log_cfg.level = 0;
        log_cfg.console_level = 0;
        log_cfg.cb = Softphone::pjsuaLogCallback;

        pjsua_media_config media_cfg;
        pjsua_media_config_default(&media_cfg);
        media_cfg.no_vad = PJ_FALSE;

        status = pjsua_init(&cfg, &log_cfg, &media_cfg);
        if (PJ_SUCCESS != status) {
            errorHandler("Cannot init PJSUA", status, true);
            return false;
        }
    }

    //add transport for standalone mode
    {
        pjsua_transport_config cfg;
        pjsua_transport_config_default(&cfg);
        cfg.port = 0;//allow PJSIP to choose an available port
        pjsua_transport_id transportId = PJSUA_INVALID_ID;
        const pjsip_transport_type_e type = _isUdpTransport ? PJSIP_TRANSPORT_UDP : PJSIP_TRANSPORT_TCP;
        status = pjsua_transport_create(type, &cfg, &transportId);
        if (PJ_SUCCESS != status) {
            errorHandler("Error creating transport", status, true);
            return false;
        }
        // read the chosen port for registrar
        pjsua_transport_info info;
        status = pjsua_transport_get_info(transportId, &info);
        if (PJ_SUCCESS != status) {
            errorHandler("Error getting transport info", status, true);
            return false;
        }
        _localPort = QString::number(info.local_name.port);
        qInfo() << "Local port" << _localPort;
        // Add local account
        status = pjsua_acc_add_local(transportId, PJ_TRUE, &_accId);
        if (PJ_SUCCESS != status) {
            errorHandler("Cannot add local account", status, true);
            return false;
        }
        status = pjsua_acc_set_online_status(_accId, PJ_TRUE);
        if (PJ_SUCCESS != status) {
            errorHandler("Cannot set online status", status, true);
            return false;
        }
    }

    //start PJSUA
    status = pjsua_start();
    if (PJ_SUCCESS != status) {
        pjsua_destroy();
        errorHandler("Error starting pjsua", status, true);
        return false;
    }

    initAudioDevicesList();

    setupAudioCodecPriority();
    setupAudioCodecParam();

#if defined (Q_OS_IOS)
    pjsua_set_no_snd_dev();
#endif

    _pjsuaStarted = true;
    return true;
}

void Softphone::release()
{
    zeroConf(false);
    if (_pjsuaStarted) {
        hangupAll();
        releaseRingTonePlayers();
        pjsua_stop_worker_threads();
        pj_status_t status = pjsua_destroy();
        if (PJ_SUCCESS != status) {
            errorHandler("Cannot destroy", status);
        } else {
            qDebug() << "PJSUA successfully destroyed";
        }
        _pjsuaStarted = false;
    }
}

bool Softphone::makeCall(int zeroConfIndex)
{
    qDebug() << "makeCall" << zeroConfIndex;

    if (PJSUA_INVALID_ID == _accId) {
        setDialedText("");
        Softphone::showMessage(tr("Invalid account"), true);
        return false;
    }
    if (!isValidZeroConfIndex(zeroConfIndex)) {
        setDialedText("");
        Softphone::showMessage(tr("Invalid user"), true);
        return false;
    }

    std::string uriBuffer;
    pj_str_t uriStr;
    if (!callUri(&uriStr, zeroConfIndex, uriBuffer)) {
        return false;
    }

    //open audio device only when needed (automatically closed when the call ends)
    setAudioDevices();

    pjsua_call_setting callSetting;
    pjsua_call_setting_default(&callSetting);
    pjsua_call_id callId = PJSUA_INVALID_ID;
    const pj_status_t status = pjsua_call_make_call(_accId, &uriStr, &callSetting,
                                                    nullptr, nullptr, &callId);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot make call", status, true);
        return false;
    }

    setActiveCall(true);
    setCurrentDeviceName(_deviceList.at(zeroConfIndex));
    if (nullptr != _activeCallModel) {
        _activeCallModel->addCall(callId, _zeroConfList.at(zeroConfIndex).uuid,
                                  _zeroConfList.at(zeroConfIndex).name);
    }
    return true;
}

bool Softphone::answer(int callId)
{
    if (PJSUA_INVALID_ID == callId) {
        qCritical() << "Invalid call ID";
        return false;
    }

    pjsua_call_setting callSetting;
    pjsua_call_setting_default(&callSetting);

    const pj_status_t status = pjsua_call_answer2(callId, &callSetting, 200,
                                                  nullptr, nullptr);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot answer call", status, true);
        return false;
    }
    return true;
}

bool Softphone::hangup(int callId)
{
    if (PJSUA_INVALID_ID == callId) {
        qCritical() << "Invalid call ID";
        return false;
    }
    qDebug() << "Hangup" << callId;

    _manualHangup = true;
    const pj_status_t status = pjsua_call_hangup(callId, 0, nullptr, nullptr);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot hangup call", status, true);
        return false;
    }
    return true;
}

bool Softphone::swap(int callId)
{
    const auto &confCids = _activeCallModel->confirmedCallsId();
    if (confCids.isEmpty()) {
        qCritical() << "No active call(s) to swap";
        return false;
    }
    qDebug() << "swap" << confCids << "with" << callId;
    for (auto confCid: confCids) {
        if (!hold(true, confCid)) {
            return false;
        }
    }
    bool rc = hold(false, callId);
    if (rc) {
        _activeCallModel->setCurrentCallId(callId);
    }
    return rc;
}

bool Softphone::merge(int callId)
{
    qDebug() << "Merge" << callId;
    if (!answer(callId)) {
        return false;
    }
    setupConferenceCall(callId);
    _activeCallModel->update();
    return true;
}

bool Softphone::hold(bool value, int callId)
{
    qDebug() << "Hold" << value;
    if (PJSUA_INVALID_ID == callId) {
        callId = _activeCallModel->currentCallId();
    }

    pj_status_t status = PJ_SUCCESS;
    if (value) {
        status = pjsua_call_set_hold(callId, nullptr);
        if (PJ_SUCCESS != status) {
            errorHandler("Cannot put call on hold", status, true);
            return false;
        }
    } else {
        status = pjsua_call_reinvite(callId, PJSUA_CALL_UNHOLD, nullptr);
        if (PJ_SUCCESS != status) {
            errorHandler("Cannot unhold call", status, true);
            return false;
        }
    }
    _activeCallModel->setCallState(callId, value ? ActiveCallModel::CallState::ON_HOLD : ActiveCallModel::CallState::CONFIRMED);
    qDebug() << "Finished to put call on hold" << value;
    return true;
}

bool Softphone::muteMicrophone(bool value, int callId)
{
    if (PJSUA_INVALID_ID == callId) {
        callId = _activeCallModel->currentCallId();
    }
    const pjsua_conf_port_id confSlot = pjsua_call_get_conf_port(callId);
    if (PJSUA_INVALID_ID == confSlot) {
        qCritical() << "Cannot get conference slot for call ID" << callId;
        return false;
    }
    qDebug() << "muteMicrophone" << value << callId;
    return setMicrophoneVolume(confSlot, value);
}

bool Softphone::muteSpeakers(bool value, int callId)
{
    if (PJSUA_INVALID_ID == callId) {
        callId = _activeCallModel->currentCallId();
    }
    const pjsua_conf_port_id confSlot = pjsua_call_get_conf_port(callId);
    if (PJSUA_INVALID_ID == confSlot) {
        qCritical() << "Cannot get conference slot for call ID" << callId;
        return false;
    }
    qDebug() << "muteSpeakers" << value << callId;
    return setSpeakersVolume(confSlot, value);
}

void Softphone::initAudioDevicesList()
{
    //refresh device list (needed when device changed notification is received)
    auto status = pjmedia_aud_dev_refresh();
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot refresh audio devices list", status, false);
    }

    unsigned devCount = PJMEDIA_AUD_MAX_DEVS;
    std::array<pjmedia_aud_dev_info, PJMEDIA_AUD_MAX_DEVS> devInfo;
    status = pjsua_enum_aud_devs(devInfo.data(), &devCount);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot get audio device info.", status, true);
        return;
    }
    qInfo() << "Found" << devCount << "audio devices";

    QVector<AudioDevices::DeviceInfo> inputDevices;
    QVector<AudioDevices::DeviceInfo> outputDevices;
    for (unsigned i = 0; i < devCount; ++i) {
        if (0 < devInfo[i].input_count) {
            const AudioDevices::DeviceInfo audioInfo{devInfo[i].name, static_cast<int>(i)};
            inputDevices.push_back(audioInfo);
            //qDebug() << "Audio input" << audioInfo.name << audioInfo.index;
        }
        if (0 < devInfo[i].output_count) {
            const AudioDevices::DeviceInfo audioInfo{devInfo[i].name, static_cast<int>(i)};
            outputDevices.push_back(audioInfo);
            //qDebug() << "Audio output" << audioInfo.name << audioInfo.index;
        }
    }
    if (inputDevices.isEmpty()) {
        Softphone::showMessage(tr("No input audio devices"), true);
    }

    _inputAudioDevices->init(inputDevices);
    const auto inDevInfo = Settings::inputAudioDeviceInfo();
    auto inDevModelIndex = _inputAudioDevices->deviceIndex(inDevInfo);
    if (AudioDevices::INVALID_MODEL_INDEX == inDevModelIndex) {
        qDebug() << "Invalid input device, using default device";
        inDevModelIndex = 0;
    }

    if (outputDevices.isEmpty()) {
        Softphone::showMessage(tr("No output audio devices"), true);
    }
    _outputAudioDevices->init(outputDevices);
    const auto outDevInfo = Settings::outputAudioDeviceInfo();
    auto outDevModelIndex = _outputAudioDevices->deviceIndex(outDevInfo);
    if (AudioDevices::INVALID_MODEL_INDEX == outDevModelIndex) {
        qDebug() << "Invalid output device, using default device";
        outDevModelIndex = 0;
    }

    //set last in settings the audio devices
    _settings->setInputAudioModelIndex(inDevModelIndex);
    _settings->setOutputAudioModelIndex(outDevModelIndex);

    qInfo() << "Found" << inputDevices.size() << "input audio devices and"
            << outputDevices.size() << "output audio devices";
}

bool Softphone::setAudioDevices()
{
    if ((nullptr == _inputAudioDevices) || (nullptr == _outputAudioDevices)) {
        qCritical() << "Cannot set audio devices";
        return false;
    }
#if defined(Q_OS_IOS)
    enableAudioSession();
#endif
    const auto captureDevInfo = _inputAudioDevices->deviceInfo();
    if (!captureDevInfo.isValid()) {
        const QString msg = QString("Invalid input audio device index %1").arg(captureDevInfo.toString());
        qCritical() << msg;
        errorHandler(msg, PJ_SUCCESS, true);
        pjsua_set_no_snd_dev();//for testing purposes only
        return false;
    }
    const auto playbackDevInfo = _outputAudioDevices->deviceInfo();
    if (!playbackDevInfo.isValid()) {
        const QString msg = QString("Invalid output audio device index %1").arg(playbackDevInfo.toString());
        qCritical() << msg;
        errorHandler(msg, PJ_SUCCESS, true);
        pjsua_set_no_snd_dev();//for testing purposes only
        return false;
    }
    pj_status_t status = pjsua_set_snd_dev(captureDevInfo.index, playbackDevInfo.index);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot set audio devices", status, true);
        return false;
    }
    qDebug() << "Finished to set audio devices: captureDev" << captureDevInfo.index <<
                ", playbackDev" << playbackDevInfo.index;
    return true;
}

bool Softphone::initRingTonePlayer(pjsua_call_id id)
{
    if (0 != _playerId.count(id)) {
        qCritical() << "Call" << id << "already has a player associated with it";
        return false;
    }

    int ringToneFileIndex = _settings->ringTonesModelIndex();
    const QString soundFileStr = _ringTonesModel->filePath(ringToneFileIndex);
    if (!QFile(soundFileStr).exists()) {
        qWarning() << "Ring tone file does not exist";
        return false;
    }
    qInfo() << "Init ringtone player" << soundFileStr;

    const std::string tmpSoundFile = soundFileStr.toStdString();
    pj_str_t soundFile;
    pj_cstr(&soundFile, tmpSoundFile.c_str());

    pjsua_player_id playerId = PJSUA_INVALID_ID;
    const pj_status_t status = pjsua_player_create(&soundFile, 0, &playerId);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot create player", status, false);
        return false;
    }
    _playerId[id] = playerId;
    return true;
}

bool Softphone::startPlayingRingTone(pjsua_call_id id)
{
    if (!initRingTonePlayer(id)) {
        qDebug() << "Cannot start playing ringtone for call" << id;
        return false;
    }
    if (0 == _playerId.count(id)) {
        qCritical() << "Cannot find a player ID for call" << id;
        return false;
    }

    pjsua_player_id playerId = _playerId[id];
    pj_status_t status = pjsua_conf_connect(pjsua_player_get_conf_port(playerId), 0);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot play ring tone to output device", status, false);
        return false;
    }
    qInfo() << "Start playing ringtone";
    return true;
}

void Softphone::stopPlayingRingTone(pjsua_call_id id)
{
    if (0 == _playerId.count(id)) {
        //qWarning() << "Cannot find player ID for call" << id;
        return;
    }
    pjsua_player_id playerId = _playerId[id];
    pj_status_t status = pjsua_conf_disconnect(pjsua_player_get_conf_port(playerId), 0);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot stop playing ring tone to output device", status, false);
        return;
    }
    qInfo() << "Stop playing ringtone";
    releaseRingTonePlayer(id);
}

bool Softphone::releaseRingTonePlayer(pjsua_call_id id)
{
    if (0 == _playerId.count(id)) {
        qCritical() << "Cannot find a player ID for call" << id;
        return false;
    }
    pjsua_player_id playerId = _playerId[id];
    pj_status_t status = pjsua_player_destroy(playerId);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot destroy ringtone player", status, false);
        return false;
    }
    _playerId.remove(id);
    qInfo() << "Release ringtone player";
    return true;
}

void Softphone::releaseRingTonePlayers()
{
    const auto &keys = _playerId.keys();
    for(const auto &id: keys) {
        releaseRingTonePlayer(id);
    }
}

void Softphone::setupAudioCodecPriority()
{
    std::array<pjsua_codec_info, MAX_CODEC_COUNT> codecInfo;
    unsigned int codecCount = codecInfo.size();
    pj_status_t status = pjsua_enum_codecs(codecInfo.data(), &codecCount);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot enum audio codecs", status);
        return;
    }
    qInfo() << "Audio codecs:" << codecCount;
    for (unsigned int n = 0; n < codecCount; ++n) {
        //disable all codecs except PCM
        const auto id = toString(codecInfo[n].codec_id);
        auto priority = codecInfo[n].priority;
        if (id.contains("opus", Qt::CaseInsensitive)) {
            priority = PJMEDIA_CODEC_PRIO_HIGHEST;
            unsigned count = 1;
            const pjmedia_codec_info *mediaCodecInfo = nullptr;
            auto *codecMgr = pjMediaCodecMgr();
            if (nullptr != codecMgr) {
                status = pjmedia_codec_mgr_find_codecs_by_id(codecMgr,
                                                             &codecInfo[n].codec_id,
                                                             &count,
                                                             &mediaCodecInfo,
                                                             nullptr);
                if ((PJ_SUCCESS == status) && (nullptr != mediaCodecInfo)) {
                    qInfo() << "Got OPUS codec info";
                    pj_memcpy(&_opusCodecInfo, mediaCodecInfo, sizeof(*mediaCodecInfo));
                } else {
                    errorHandler("Cannot find codec by ID", status, false);
                }
            }
        } else {
            priority = PJMEDIA_CODEC_PRIO_DISABLED;
        }
        setAudioCodecPriority(id, priority);
        //qInfo() << id << priority;
    }
}

pjmedia_codec_mgr* Softphone::pjMediaCodecMgr()
{
    auto *mediaEndp = pjsua_get_pjmedia_endpt();
    if (nullptr == mediaEndp) {
        qCritical() << "Cannot get media end point";
        return nullptr;
    }
    auto *codecMgr = pjmedia_endpt_get_codec_mgr(mediaEndp);
    if (nullptr == codecMgr) {
        qCritical() << "Cannot get codec manager";
    }
    return codecMgr;
}

bool Softphone::setupAudioCodecParam()
{
    if (!_pjsuaStarted) {
        return false;//nothing to do
    }
    if (nullptr == _settings) {
        qCritical() << "Cannot get settings params";
        return false;
    }
    //setup only OPUS codec params
    pjmedia_codec_opus_config cfg;
    auto status = pjmedia_codec_opus_get_config(&cfg);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot get OPUS codec config", status, true);
        return false;
    }

    cfg.sample_rate = _settings->sampRateKhz() * 1000;
    cfg.frm_ptime = _settings->frameSizeMs();
    cfg.bit_rate = _settings->bitrateKbps() * 1000;
    cfg.cbr = !_settings->isVariableBitRate();

    auto *codecMgr = pjMediaCodecMgr();
    if (nullptr == codecMgr) {
        return false;
    }
    pjmedia_codec_param param;
    status = pjmedia_codec_mgr_get_default_param(codecMgr, &_opusCodecInfo, &param);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot get default config", status, false);
    }

    status = pjmedia_codec_opus_set_default_param(&cfg, &param);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot set OPUS codec config", status, true);
        return false;
    }
    qInfo() << "Setup OPUS codec";
    return true;
}

void Softphone::errorHandler(const QString &title, pj_status_t status, bool emitSignal)
{
    QString fullError = title;
    if (PJ_SUCCESS != status) {
        std::array<char, ERROR_MSG_SIZE> message;
        pj_strerror(status, message.data(), message.size());
        fullError.append(":").append(message.data());
    }
    if (emitSignal) {
        Softphone::showMessage(fullError, true);
    }
}

bool Softphone::setMicrophoneVolume(pjsua_conf_port_id portId, bool mute)
{
    //microphone volume
    const qreal microphoneLevel = mute ? 0 : _settings->microphoneVolume();
    qDebug() << "Mic level" << microphoneLevel;
    const auto status = pjsua_conf_adjust_tx_level(portId,
                                                    static_cast<float>(microphoneLevel));
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot adjust tx level", status);
        return false;
    }
    return true;
}

bool Softphone::setSpeakersVolume(pjsua_conf_port_id portId, bool mute)
{
    //speakers volume
    const qreal speakersLevel = mute ? 0 : _settings->speakersVolume();
    qDebug() << "Speaker level" << speakersLevel;
    const auto status = pjsua_conf_adjust_rx_level(portId,
                                                    static_cast<float>(speakersLevel));
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot adjust rx level", status);
        return false;
    }
    return true;
}

void Softphone::hangupAll()
{
    const auto ids = _activeCallModel->confirmedCallsId(true);
    qDebug() << "hangupAll" << ids.count();
    for (auto id: ids) {
        hangup(id);
    }
    if (!ids.isEmpty()) {
        setActiveCall(false);
    }
}

bool Softphone::callUri(pj_str_t *uri, int zeroConfIndex, std::string &uriBuffer)
{
    const auto serverUrl = _zeroConfList.at(zeroConfIndex).address;
    const auto serverPort = _zeroConfList.at(zeroConfIndex).port;
    const QString sipUri = "\"" + deviceName() + "\" <sip:" + serverUrl +
            this->serverPort(serverPort) + currentTransport() +
            QString(";uuid=\"%1\">").arg(_deviceUuid);
    uriBuffer = sipUri.toStdString();
    const char *uriPtr = uriBuffer.c_str();
    const pj_status_t status = pjsua_verify_sip_url(uriPtr);
    if (PJ_SUCCESS != status) {
        errorHandler("URI verification failed", status, true);
        return false;
    }
    if (nullptr != uri) {
        pj_cstr(uri, uriPtr);
    }
    qInfo() << "Call URI" << sipUri;
    return true;
}

void Softphone::connectCallToSoundDevices(pjsua_conf_port_id confPortId)
{
    qDebug() << "Connect call conf port" << confPortId << "to sound devices";

    //ajust volume level
    setMicrophoneVolume(confPortId);
    setSpeakersVolume(confPortId);

    pj_status_t status = pjsua_conf_connect(confPortId, 0);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot connect conf slot to playback slot", status,
                     false);
    }
    status = pjsua_conf_connect(0, confPortId);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot connect capture slot to conf slot", status,
                     false);
    }
}

void Softphone::disconnectCallFromSoundDevices(pjsua_conf_port_id confPortId)
{
    pjsua_conf_disconnect(confPortId, 0);
    pjsua_conf_disconnect(0, confPortId);
}

void Softphone::setupConferenceCall(pjsua_call_id callId)
{
    const auto callCount = pjsua_call_get_count();
    qDebug() << "setupConferenceCall" << callCount;
    if (1 < callCount) {
        pjsua_conf_port_id confPortId = pjsua_call_get_conf_port(callId);
        if (PJSUA_INVALID_ID == confPortId) {
            qCritical() << "Cannot get current call conf port";
            return;
        }
        const QVector<int> confCalls = _activeCallModel->confirmedCallsId();
        qDebug() << "Conference call detected, confirmed calls" << confCalls.size();
        for (auto otherCallId: confCalls) {
            if (callId == otherCallId) {
                continue;
            }
            const pjsua_conf_port_id otherCallConfPort = pjsua_call_get_conf_port(otherCallId);
            if (PJSUA_INVALID_ID == otherCallConfPort) {
                qWarning() << "Cannot get conference slot of call ID" << otherCallId;
                continue;
            }
            pj_status_t status = pjsua_conf_connect(confPortId, otherCallConfPort);
            if (PJ_SUCCESS != status) {
                errorHandler("Cannot connect current conf slot to other", status, false);
            }
            status = pjsua_conf_connect(otherCallConfPort, confPortId);
            if (PJ_SUCCESS != status) {
                errorHandler("Cannot connect other conf slot to current", status, false);
            }
        }
    }
}

void Softphone::onCurrentUserTimeout()
{
    unsigned txLevel = 0;
    unsigned rxLevel = 0;
    std::array<pjsua_conf_port_id, PJSUA_MAX_CONF_PORTS> confId;
    unsigned count = confId.size();
    auto status = pjsua_enum_conf_ports(confId.data(), &count);
    if (PJ_SUCCESS != status) {
        errorHandler("Cannot get conf ports", status, false);
        return;
    }
    unsigned txLevelCurr = 0;
    unsigned rxLevelCurr = 0;
    pjsua_conf_port_info confPortInfo;
    for (unsigned i = 0; i < count; i++) {
        status = pjsua_conf_get_port_info(confId[i], &confPortInfo);
        if (PJ_SUCCESS == status) {
            status = pjsua_conf_get_signal_level(confId[i], &txLevelCurr, &rxLevelCurr);
            if (PJ_SUCCESS == status) {
                if (confPortInfo.slot_id == 0) {
                    txLevel = rxLevelCurr * (confPortInfo.rx_level_adj > 0 ? 1 : 0);
                } else {
                    rxLevelCurr = confPortInfo.rx_level_adj > 0 ? rxLevelCurr : 0;
                    if (rxLevelCurr > rxLevel) {
                        rxLevel = rxLevelCurr;
                    }
                }
            }
        }
    }
    setTxLevel(txLevel);
    setRxLevel(rxLevel);
    //qDebug() << "Level: tx" << txLevel << ", rx" << rxLevel;
}

void Softphone::zeroConf(bool start)
{
    qDebug() << "zeroConf" << start;
    if (start) {
        if (!_zeroConf.publishExists()) {
            startPublish();
        }
        if (!_zeroConf.browserExists()) {
            _zeroConf.startBrowser(ZERO_CONF_TYPE);
            qInfo() << "Start browser";
        }
    } else {
        _zeroConf.stopServicePublish();
        _zeroConf.stopBrowser();
        qInfo() << "Stop browser";
    }
}

void Softphone::onMicrophoneVolumeChanged()
{
    qDebug() << "onMicrophoneVolumeChanged";
    const auto currentCallId = _activeCallModel->currentCallId();
    if ((PJSUA_INVALID_ID == currentCallId) ||
            (PJ_FALSE == pjsua_call_is_active(currentCallId))) {
        qDebug() << "No active call";
        return;
    }
    const pjsua_conf_port_id confSlot = pjsua_call_get_conf_port(currentCallId);
    if (PJSUA_INVALID_ID != confSlot) {
        setMicrophoneVolume(confSlot);
    } else {
        qCritical() << "Cannot get conference slot of call ID" << currentCallId;
    }
}

void Softphone::onSpeakersVolumeChanged()
{
    qDebug() << "onSpeakersVolumeChanged";
    const auto currentCallId = _activeCallModel->currentCallId();
    if ((PJSUA_INVALID_ID == currentCallId) ||
            (PJ_FALSE == pjsua_call_is_active(currentCallId))) {
        qDebug() << "No active call";
        return;
    }
    const pjsua_conf_port_id confSlot = pjsua_call_get_conf_port(currentCallId);
    if (PJSUA_INVALID_ID != confSlot) {
        setSpeakersVolume(confSlot);
    } else {
        qCritical() << "Cannot get conference slot of call ID" << currentCallId;
    }
}

void Softphone::checkMediaType(const pjsua_call_info &callInfo)
{
    unsigned cnt = 0;
    unsigned cntSrtp = 0;
    for (unsigned i = 0; i < callInfo.media_cnt; i++) {
        if (callInfo.media[i].dir != PJMEDIA_DIR_NONE &&
                (callInfo.media[i].type == PJMEDIA_TYPE_AUDIO || callInfo.media[i].type == PJMEDIA_TYPE_VIDEO)) {
            ++cnt;
            pjsua_call_info call_info_stub;
            if (pjsua_call_get_info(callInfo.id, &call_info_stub) == PJ_SUCCESS) {

                pjmedia_transport_info t;
                if (pjsua_call_get_med_transport_info(callInfo.id, callInfo.media[i].index, &t) == PJ_SUCCESS) {
                    for (unsigned j = 0; j < t.specific_info_cnt; j++) {
                        if (t.spc_info[j].buffer[0]) {
                            switch (t.spc_info[j].type) {
                            case PJMEDIA_TRANSPORT_TYPE_SRTP:
                                ++cntSrtp;
                                break;
                            case PJMEDIA_TRANSPORT_TYPE_ICE:
                                break;
                            default:
                                ;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Softphone::showMessage(const QString &msg, bool error)
{
    if (nullptr != _instance) {
        qDebug() << msg << error;
        emit _instance->showMessageDialog(msg, error, false);
    }
}

QString Softphone::deviceName()
{
    return QHostInfo::localHostName();
}

QString Softphone::generateDeviceUuid()
{
    QString uuid;
#ifdef Q_OS_IOS
    uuid = deviceUuid();
#else
    const auto allIf = QNetworkInterface::allInterfaces();
    for (const auto &it: allIf) {
        const auto type = it.type();
        if (it.isValid() &&
                !(it.flags() & QNetworkInterface::IsLoopBack) &&
                ((QNetworkInterface::Ethernet == type) || (QNetworkInterface::Wifi == type))) {
            const auto &mac = it.hardwareAddress();
            if (!mac.isEmpty() && !mac.endsWith("00:00:00")) {
                uuid = QCryptographicHash::hash(mac.toUtf8(), QCryptographicHash::Sha256).toHex();
                break;
            }
        }
    }
#endif
    if (uuid.isEmpty()) {
        qWarning() << "Invalid UUID, using random UUID";
        uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    qInfo() << "Device UUID" << uuid;
    return uuid;
}

void Softphone::updateCurrentDeviceName(const QString &name, bool add)
{
    if (name.isEmpty()) {
        qWarning() << "Cannot update current device name";
        return;
    }
    if (add) {
        if (_currentDeviceName.isEmpty()) {
            _currentDeviceName = name;
        } else {
            _currentDeviceName += ", " + name;
        }
    } else {
        if (_currentDeviceName == name) {
            _currentDeviceName.clear();
        } else if (_currentDeviceName.startsWith(name)) {
            _currentDeviceName.remove(name + ", ");
        } else {
            _currentDeviceName.remove(", " + name);
        }
    }
    emit currentDeviceNameChanged();
}

void Softphone::startPublish()
{
    if (_localPort.isEmpty()) {
        emit showMessageDialog(tr("Registrar port is not set"), true, false);
        return;
    }
    _zeroConf.clearServiceTxtRecords();
    _zeroConf.addServiceTxtRecord(ZERO_CONF_SIP_PORT, _localPort);
     _zeroConf.addServiceTxtRecord(ZERO_CONF_DEVICE_UUID, _deviceUuid);
    const auto itemName = deviceName();
    _zeroConf.startServicePublish(itemName.toUtf8(), ZERO_CONF_TYPE,
                                  nullptr, ZERO_CONF_PORT);
    qInfo() << "Start publish service" << itemName;
}

void Softphone::addService(QZeroConfService item)
{
    qDebug() << "Added service:" << item;

    //filter own service
    const auto addr = item->ip().toString();
    const auto devAddr = QHostAddress(addr);
    if (devAddr.isLoopback()) {
        qDebug() << "Skip loopback address";
        return;
    }
    //filter class B Private APIPA Range: 169.254.0.0 to 169.254.255.255
    if (devAddr.isLinkLocal()) {
        qDebug() << "Skip Class B Private APIPA address" << addr;
        return;
    }
    //filter own address
    const auto addrList = QNetworkInterface::allAddresses();
    if (addrList.contains(devAddr)) {
        qDebug() << "Skip own address";
        return;
    }

    const auto name = item->name();
    const auto port = item->txt().value(ZERO_CONF_SIP_PORT);
    const auto uuid = item->txt().value(ZERO_CONF_DEVICE_UUID);
    ZeroConfItem zeroConfItem{ name, addr, port, uuid };
    if (zeroConfItem.uuid.isEmpty()) {
        qWarning() << "Invalid UUID";
        return;
    }

    //update existing service
    bool found = false;
    for (int i = 0; i < _zeroConfList.size(); ++i) {
        if (_zeroConfList.at(i).uuid == uuid) {
            qDebug() << "Update existing service";
            _zeroConfList[i] = zeroConfItem;
            _deviceList[i] = name;
            emit trayIconMessage(tr("Update service ") + name);
            found = true;
            break;
        }
    }

    if (!found) {
        _zeroConfList << zeroConfItem;
        _deviceList << name;
        emit trayIconMessage(tr("New service ") + name);
    }
    emit deviceListChanged();
}

void Softphone::removeService(QZeroConfService item)
{
    qDebug() << "Zero conf remove service:" << item;
    if (!item.isNull()) {
        const auto uuid = item->txt().value(ZERO_CONF_DEVICE_UUID);
        removeServiceWithUuid(uuid);
    } else {
        qWarning() << "Cannot remove service";
    }
}

void Softphone::removeServiceWithUuid(const QString &uuid)
{
    qDebug() << "Remove service" << uuid;
    if (uuid.isEmpty()) {
        qWarning() << "Invalid UUID";
        return;
    }
    for (int i = 0; i < _zeroConfList.size(); ++i) {
        if (uuid == _zeroConfList.at(i).uuid) {
            _zeroConfList.removeAt(i);
            const auto name = _deviceList.at(i);
            _deviceList.removeAt(i);
            emit deviceListChanged();
            qInfo() << "Service removed" << uuid;
            emit trayIconMessage(tr("Removed service ") + name);
            return;
        }
    }
}

void Softphone::updateService(QZeroConfService item)
{
    qDebug() << "Update service:" << item;
    if (item.isNull()) {
        qWarning() << "Cannot update service";
        return;
    }
    const auto uuid = item->txt().value(ZERO_CONF_DEVICE_UUID);
    if (uuid.isEmpty()) {
        qWarning() << "Invalid UUID";
        return;
    }
    for (int i = 0; i < _zeroConfList.size(); ++i) {
        if (uuid == _zeroConfList.at(i).uuid) {
            _zeroConfList[i].name = item->name();
            _zeroConfList[i].address = item->ip().toString();
            _zeroConfList[i].port = item->txt().value(ZERO_CONF_SIP_PORT);
            return;
        }
    }
}

void Softphone::updateServiceWithItem(const ZeroConfItem &zcItem)
{
    qDebug() << "Update service:" << zcItem.uuid;
    if (zcItem.uuid.isEmpty()) {
        qWarning() << "Invalid UUID";
        return;
    }
    bool foundItem = false;
    for (int i = 0; i < _zeroConfList.size(); ++i) {
        if (zcItem.uuid == _zeroConfList.at(i).uuid) {
            _zeroConfList[i] = zcItem;
            _deviceList[i] = zcItem.name;
            foundItem = true;
        }
    }
    if (!foundItem) {
        _zeroConfList << zcItem;
        _deviceList << zcItem.name;
    }
    emit deviceListChanged();
}

void Softphone::extractAddressUserName(ZeroConfItem &zcItem, const QString &remoteContact,
                                       const QString &localInfo)
{
    const static QRegularExpression addrRe("<sip:([\\d|\\.]+):(\\d+).+>");
    const static QRegularExpression userRe("\"(.+)\"\\s*<sip:.+;uuid=\"(.+)\".*");

    zcItem.clear();

    if (!remoteContact.isEmpty()) {
        QRegularExpressionMatch match = addrRe.match(remoteContact);
        if (match.hasMatch()) {
            zcItem.address = match.captured(1);
            zcItem.port = match.captured(2);
        } else {
            qWarning() << "No remote contact match" << remoteContact;
        }
    } else {
        qWarning() << "Remote contact is empty";
    }
    if (!localInfo.isEmpty()) {
        QRegularExpressionMatch match = userRe.match(localInfo);
        if (match.hasMatch()) {
            zcItem.name = match.captured(1);
            zcItem.uuid = match.captured(2);
        } else {
            qWarning() << "No local info match" << localInfo;
        }
        if (zcItem.name.isEmpty()) {
            zcItem.name = tr("Unknown");
        }
    } else {
        qWarning() << "Local info is empty";
    }
    qDebug() << "Matched" << zcItem.name << zcItem.address << zcItem.port << zcItem.uuid;
}

void Softphone::onRetryConnection()
{
    //check the connection timeout
    if (_retryElapsedTimer.elapsed() > (1000 * _settings->lostConnTimeoutSec())) {
        qInfo() << "Lost connection timeout expired";
        resetElapsedTimer();
        emit showMessageDialog(tr("Lost connection timeout expired. Please try a new session "
                    "when both devices are in range of each other."), true, false);
        return;
    }

    if (!_currentDeviceName.isEmpty()) {
        for (int i = 0; i < _zeroConfList.size(); ++i) {
            if (_zeroConfList.at(i).name == _currentDeviceName) {
                makeCall(i);
                break;
            }
        }
    } else {
        qWarning() << "Nothing to do";
    }
}

void Softphone::resetElapsedTimer()
{
    _retryElapsedTimer.invalidate();
    _instance->setShowBusy(false);
}

void Softphone::raiseWindow()
{
    qDebug() << "Raise window";
    if (nullptr != _mainForm) {
        if (!QMetaObject::invokeMethod(_mainForm, "raiseWindow",
                                       Qt::AutoConnection)) {
            qCritical() << "Cannot call raiseWindow method";
        }
    } else {
        qCritical() << "mainForm object is NULL";
    }
}
