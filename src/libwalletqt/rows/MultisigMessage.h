//
// Created by user on 1/3/24.
//

#ifndef FEATHER_MULTISIGMESSAGE_H
#define FEATHER_MULTISIGMESSAGE_H

#include <QObject>
#include <QDateTime>

//uint32_t id;
//message_type type;
//message_direction direction;
//std::string content;
//uint64_t created;
//uint64_t modified;
//uint64_t sent;
//uint32_t signer_index;
//crypto::hash hash;
//message_state state;
//uint32_t wallet_height;
//uint32_t round;
//uint32_t signature_count;
//std::string transport_id;

class MultisigMessage : public QObject
{
    Q_OBJECT

public:
    quint32 id;
    QString type;
    QString direction;
    std::string content;
    QDateTime created;
    QDateTime modified;
    QDateTime sent;
    quint32 signer_index;
    QString signer;
    QString hash;
    QString state;
    quint32 wallet_height;
    quint32 round;
    quint32 signature_count;
    QString transport_id;


private:
    explicit MultisigMessage(QObject *parent);

private:
    friend class MultisigMessageStore;
};


#endif //FEATHER_MULTISIGMESSAGE_H
