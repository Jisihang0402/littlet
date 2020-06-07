#pragma once

#include <QObject>
#include <QHash>
#include <QMultiHash>
#include <QList>

typedef QList<QByteArray> ReciveMessage;

// ���ݽ�����
class ReciverSignaler : public QObject
{
    Q_OBJECT;

public:
    ReciverSignaler(QObject* sender_obj, const char* slot);

    QObject *object()
    {
        return obj_;
    }

    QString sloter()
    {
        return slot_;
    }

signals:
    void signal_MessageRecived(qint64 send_no, QByteArray data);

private:
    // �����߶���
    QObject*    obj_ = nullptr;

    // �źŲ�
    QString     slot_;
};

// ĳһ�������͵����ݽ�����
class HeadSubscriber : public QObject
{
    Q_OBJECT;

public:
    HeadSubscriber(QObject* sender_obj, const char* slot);

    QObject *object()
    {
        return obj_;
    }

    QString sloter()
    {
        return slot_;
    }

signals:
    void signal_MessageRecived(int head, QByteArray data);

private:
    // �����߶���
    QObject*    obj_ = nullptr;

    // �źŲ�
    QString     slot_;
};

/**
 *	�첽zmq�����ݽ��ն���	
 *
**/
class KRecvQueue : public QObject
{
    Q_OBJECT;

public:
    KRecvQueue();
    ~KRecvQueue();

    /**
     *	���һ��������	
     *
     *	@param
     *		-[in]
     *          obj     ������
     *          slot    ����Ϣʱ�����slot
     *      -[out]
     *          send_id ����ֶ�һ��Ҫд�뵽����������
    **/
    qint64 AddReciver(QObject* obj, const char* slot, QByteArray& send_id);

    // ĳһ���ͷ�Ľ�����
    // �����������е� json ["head"] == head_noʱ�����͵�����
    bool SubscribeHead(QObject* obj, const char* slot, int head_no);

    /**
     *	��Ϣ�յ���	
     *
     *	@return
     *		true    ��Ϣ��ȷ��
     *
    **/
    bool MessageRecevied(const QList<QByteArray>& data);

    bool UnregisterObject(QObject* obj);

protected:
    /**
     *	����Ѿ�ע�ᣬ����nullptr	
     *
    **/
    ReciverSignaler* RegisterReciver(QObject *obj, const char* slot);

private:
    // ĳһ��head�Ľ�����
    // һ����Ϣ�ſ��Ա����������Ԥ��
    QMultiHash<int, HeadSubscriber*>  head_subs_;

    // �Ѿ�ע��Ľ�����
    //QMultiHash<QObject*, QString> recvs_;
    QList<ReciverSignaler*> recvs_;
};

