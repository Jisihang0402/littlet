#include "KRecvQueue.h"
#include <QDebug>
#include "deps/rjson/inc.h"

ReciverSignaler::ReciverSignaler(QObject* sender_obj, const char* sloter)
    :QObject(nullptr), obj_(sender_obj), slot_(sloter)
{
    // �������յ���ʱ��ἤ���ź�
    sender_obj->connect(this, SIGNAL(signal_MessageRecived(qint64, QByteArray)),
        sloter, Qt::QueuedConnection);
}

HeadSubscriber::HeadSubscriber(QObject* sender_obj, const char* sloter)
    :QObject(nullptr), obj_(sender_obj), slot_(sloter)
{
    // �������յ���ʱ��ἤ���ź�
    sender_obj->connect(this, SIGNAL(signal_MessageRecived(int, QByteArray)),
        sloter, Qt::QueuedConnection);
}

//////////////////////////////////////////////////////////////////////////
KRecvQueue::KRecvQueue()
{
}

KRecvQueue::~KRecvQueue()
{
    // �����ڴ�
    for (auto *p : recvs_)
    {
        delete p;
    }

    for (auto i = head_subs_.begin(); i != head_subs_.end(); ++i)
    {
        delete i.value();
    }
}

qint64 KRecvQueue::AddReciver(QObject* obj, const char* slot, QByteArray& send_id)
{
    // ÿһ����������Ψһ�����
    static qint64 _send_no = 0;

    // ��������Զ��ۼ�
    ++_send_no;

    if ((nullptr != obj) && (nullptr != slot))
    {
        ReciverSignaler* reciver = RegisterReciver(obj, slot);

#ifdef _WIN64
        send_id = QString("%1;%2").arg((qlonglong)reciver).arg(_send_no).toUtf8();
#else
        send_id = QString("%1;%2").arg((long)reciver).arg(_send_no).toUtf8();
#endif
    }
    else
    {
        send_id = QString("0;%1").arg(_send_no).toUtf8();
    }

    return _send_no;
}

bool KRecvQueue::MessageRecevied(const QList<QByteArray>& message)
{
    // ��3֡��Ϣ��������ǣ�����
    if (message.size() != 4)
        return false;

#ifdef _DEBUG
    qDebug() << QStringLiteral("�첽���ݽ��ն����յ����ݣ�");
    qDebug() << QStringLiteral("------------------");
    for (auto i = message.begin(); i != message.end(); ++i)
    {
        qDebug() << *i;
    }
    qDebug() << QStringLiteral("------------------");
#endif

    // �����
    // 0, ��֡�����ã�����

    // 1 , ReciverSignalerָ�� ; �������
    QStringList lst = QString(message[1]).split(';');
    if (lst.size() != 2)
    {
        Q_ASSERT(false);
        return false;
    }

    // 1, recvier
#ifdef _WIN64
    ReciverSignaler* reciver_signaler = (ReciverSignaler*)(lst[0].toLongLong());
#else
    ReciverSignaler* reciver_signaler = (ReciverSignaler*)(lst[0].toLong());
#endif

    // ����������ɾ�����������
    //      ���ڵķ���������������һЩ����
    // recvs_.takeAt(recvs_.indexOf(p));

    // 2, ������
    // ....�����ã�����

    // qDebug() << QStringLiteral("-------->������յ������ݣ����У�") << items_.size();
    
    // 3, ���ص���Ϣ����
    // 2014/9/3
    //      ��Ϣ�Ľ����߲���������ķ�����
    //      �˴����ݰ�ͷ�����ٷַ�

    // �������
    //////////////////////////////////////////////////////////////////////////
    // -- Э�����������
    RJsonDoc doc(message[3], false);
    bool bhandled = false;
    if (!doc.HasParseError() && doc.HasMember("head"))
    {
        // ���ݰ�ͷ���зַ�����
        int head = doc["head"].GetInt();
        for (HeadSubscriber* h : head_subs_.values(head))
        {
            Q_ASSERT(nullptr != h);
            if (nullptr != h)
            {
                bhandled = true;
                emit h->signal_MessageRecived(head, message[3]);
            }
        }
    }

    if (!bhandled && reciver_signaler)
    {
        emit reciver_signaler->signal_MessageRecived(lst[1].toLongLong(), message[3]);
    }

    return true;
}

ReciverSignaler* KRecvQueue::RegisterReciver(QObject *obj, const char* slot)
{
    for (ReciverSignaler* r : recvs_)
    {
        if ((r->object() == obj) && (r->sloter().compare(slot) == 0))
        {
            return r;
        }
    }

    ReciverSignaler* r = new ReciverSignaler(obj, slot);
    recvs_.push_back(r);
    return r;
}

bool KRecvQueue::UnregisterObject(QObject* obj)
{
    for (auto i = head_subs_.begin(); i != head_subs_.end();)
    {
        auto *p = i.value();
        if (p->object() == obj)
        {
            delete p;
            i = head_subs_.erase(i);
        }
        else
        {
            ++i;
        }
    }

    for (auto i = recvs_.begin(); i != recvs_.end();)
    {
        auto *p = *i;
        if (p->object() == obj)
        {
            i = recvs_.erase(i);
            delete p;
        }
        else
        {
            ++i;
        }
    }
    return true;
}

bool KRecvQueue::SubscribeHead(QObject* obj, const char* slot, int head_no)
{
    HeadSubscriber* p = head_subs_.value(head_no);
    if (nullptr != p)
    {
        qWarning() << QStringLiteral("Object�Ѿ�Ԥ����Ϣ�ţ�")<< head_no;

        if (p->object() == obj  && (p->sloter().compare(slot) == 0))
        {
            Q_ASSERT(false);
            return false;
        }

        // return false;
    }

    head_subs_.insert(head_no, new HeadSubscriber(obj, slot));

    return true;
}
