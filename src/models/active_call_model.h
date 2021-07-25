#pragma once

#include "pj/config_site.h"
#include "pjsua.h"
#include <QHash>
#include <QDateTime>
#include <QAbstractListModel>

class ActiveCallModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int callCount READ callCount NOTIFY callCountChanged)

public:
    enum class CallState { UNKNOWN, PENDING, CONFIRMED, ON_HOLD };
    enum CallHistoryRoles {
        CallId = Qt::UserRole+1,
        IsCurrentCall
    };

    explicit ActiveCallModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int,QByteArray> roleNames() const;

    void addCall(int callId, const QString &address);
    void setCallState(int callId, CallState callState);
    void removeCall(int callId);

    QVector<int> confirmedCallsId(bool includePending = false) const;
    bool isConference() const { return 1 < confirmedCallsId().size(); }

    bool isEmpty() const { return _callInfo.isEmpty(); }
    int callCount() const { return _callInfo.size(); }

    void setCurrentCallId(int callId);
    int currentCallId() const { return _currentCallId; }
    void update(bool active = true);

    bool isConfirmedCall(int callId);

signals:
    void callCountChanged();
    void currentUserNameChanged();
    void activeCallChanged(bool value);
    void unholdCall(int callId);

private:
    struct CallInfo {
        QDateTime callStartTime;
        bool incoming = true;
        CallState callState = CallState::UNKNOWN;
        QString remoteAddress;
    };
    bool isCurrentCall(int callId) const;
    bool isValidIndex(int index) const {
            return ((index >= 0) && (index < _callOrder.count()));
        }
    QHash<int, CallInfo> _callInfo;//key is the call ID
    QVector<int> _callOrder;
    int _currentCallId = PJSUA_INVALID_ID;
};
