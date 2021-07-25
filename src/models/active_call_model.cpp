#include "active_call_model.h"
#include <QDebug>

ActiveCallModel::ActiveCallModel(QObject *parent) : QAbstractListModel(parent)
{
}

int ActiveCallModel::rowCount(const QModelIndex& /*parent*/) const
{
    return callCount();
}

QVariant ActiveCallModel::data(const QModelIndex &index, int role) const
{
    if (!isValidIndex(index.row())) {
        qCritical() << "Invalid model index";
        return QVariant();
    }
    const auto &cid = _callOrder.at(index.row());
    if (!_callInfo.contains(cid)) {
        qCritical() << "Invalid model call ID";
        return QVariant();
    }
    //const auto &callInfo = _callInfo.value(cid);
    QVariant out;
    switch (role) {
    case CallId:
        out = cid;
        break;
    case IsCurrentCall:
        out = isCurrentCall(cid);
        break;
    default:
        qCritical() << "unknown role" << role;
    }
    return out;
}

QHash<int,QByteArray> ActiveCallModel::roleNames() const
{
    static const auto roles = QHash<int, QByteArray> {
        { CallId, "callId" },
        { IsCurrentCall, "isCurrentCall" }
    };
    return roles;
}

void ActiveCallModel::addCall(int callId, const QString &deviceUuid, const QString &name)
{
    if (PJSUA_INVALID_ID == callId) {
        qCritical() << "Invalid call ID";
        return;
    }
    if (deviceUuid.isEmpty()) {
        qWarning() << "Dev. UUID is empty";
    }
    qDebug() << "addCall" << callId;
    emit layoutAboutToBeChanged();
    if (!_callInfo.contains(callId)) {
        CallInfo info;
        info.callStartTime = QDateTime::currentDateTime();
        info.callState = CallState::PENDING;
        info.name = name;
        info.deviceUuid = deviceUuid;
        _callInfo[callId] = info;
        _callOrder.append(callId);
        emit callCountChanged();
    } else if (!deviceUuid.isEmpty()) {
        _callInfo[callId].name = name;
        _callInfo[callId].deviceUuid = deviceUuid;
    }
    emit layoutChanged();
    emit currentUserNameChanged();
}

void ActiveCallModel::setCallState(int callId, ActiveCallModel::CallState callState)
{
    if (_callInfo.contains(callId)) {
        emit layoutAboutToBeChanged();
        _callInfo[callId].callState = callState;
        emit layoutChanged();
    } else {
        qWarning() << "Cannot set call state" << callId;
    }
}

void ActiveCallModel::removeCall(int callId)
{
    emit layoutAboutToBeChanged();
    if (_callInfo.contains(callId)) {
        _callInfo.remove(callId);
        _callOrder.removeAll(callId);
        emit callCountChanged();
        qInfo() << "Removed call ID" << callId;
    } else {
        qWarning() << "Cannot find call ID" << callId;
    }
    if (!_callInfo.isEmpty()) {
        callId =  _callOrder.last();
        if (CallState::ON_HOLD == _callInfo.value(callId).callState) {
            emit unholdCall(callId);
        }
    } else {
        callId = PJSUA_INVALID_ID;
    }
    setCurrentCallId(callId);
    emit layoutChanged();
}

QString ActiveCallModel::deviceUuid(int callId) const
{
    QString uuid;
    if (_callInfo.contains(callId)) {
        uuid = _callInfo.value(callId).deviceUuid;
    }
    return uuid;
}

QString ActiveCallModel::name(int callId) const
{
    QString name;
    if (_callInfo.contains(callId)) {
        name = _callInfo.value(callId).name;
    }
    return name;
}

QVector<int> ActiveCallModel::confirmedCallsId(bool includePending) const
{
    QVector<int> ids;
    for (auto it = _callInfo.constBegin(); it != _callInfo.constEnd(); ++it) {
        const auto &info = it.value();
        bool cond =  (CallState::CONFIRMED == info.callState);
        if (includePending) {
            cond = cond || (CallState::PENDING == info.callState);
        }
        if (cond) {
            ids.append(it.key());
        }
    }
    return ids;
}

void ActiveCallModel::setCurrentCallId(int callId)
{
    if (_currentCallId != callId) {
        _currentCallId = callId;
        update(PJSUA_INVALID_ID != callId);
    }
}

bool ActiveCallModel::isCurrentCall(int callId) const
{
    return (_callInfo.contains(callId) &&
            (CallState::CONFIRMED == _callInfo.value(callId).callState));
}

void ActiveCallModel::update(bool active)
{
    emit currentUserNameChanged();
    emit activeCallChanged(active);
}

bool ActiveCallModel::isConfirmedCall(int callId)
{
    for (auto it = _callInfo.constBegin(); it != _callInfo.constEnd(); ++it) {
        const auto &info = it.value();
        if ((callId == it.key()) && (CallState::CONFIRMED == info.callState)) {
            return true;
        }
    }
    return false;
}
