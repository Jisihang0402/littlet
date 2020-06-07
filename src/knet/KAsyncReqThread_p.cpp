#include "KAsyncReqThread_p.h"
#include "include/kconfig.h"
#include "include/base.h"
#include "awaker.h"

#define AWAKE_CONNECTION_SERVERNAME "__AWAKE_SOCKET_SERVER"
#define AWAKE_CONNECTION_ADDR "inproc://awakeconnection"

#define AWAKE_REASON_POSTMSG "postmsg"
#define AWAKE_REASON_ADDCONNECTION "newconn"
#define AWAKE_REASON_SHUTDOWN "shutdown"
#define AWAKE_REASON_ADDHEADSUBSCRIBER "new_head_subscriber"
#define AWAKE_REASON_UNREGISTEROBJECT "object_unreg"

//////////////////////////////////////////////////////////////////////////
KAsyncReqThreadPrivate::KAsyncReqThreadPrivate()
{
    
}

KAsyncReqThreadPrivate::~KAsyncReqThreadPrivate()
{
//    Q_ASSERT(nullptr == ctx_);
}

qint64 KAsyncReqThreadPrivate::PostRequest(QObject* sender_obj,
    const char* slot,
    const QString& connection_name, 
    const QString& service_name, 
    const QByteArray& data)
{
    static const QByteArray NULL_BYTEARRAY;

    _SendingItem* p = new _SendingItem;
    p->connection_name = connection_name;
    p->sender_ = sender_obj;

#ifdef _DEBUG
    p->service_name = service_name;
#endif

    // ��4֡��Ϣ
    // 1, 0byte �մ�
    // 2, ������
    // 3, �����߱�ʶ��
    // 4, ��Ϣ����

    // 1,
    p->data.push_back(NULL_BYTEARRAY);
    // 2, 
    p->data.push_back(service_name.toUtf8());
    // 3, �������
    QByteArray send_id;
    queue_locker_.lock();
    qint64 send_no = recvs_.AddReciver(sender_obj, slot, send_id);
    queue_locker_.unlock();

#ifdef _DEBUG
    p->no_ = send_no;
#endif

    p->data.push_back(send_id);

    // ���´����ڱ��ز����ǿ��Եģ��ܹ�д��ȥ�����ܹ���������
    // ���Ƿ��͵�Զ�̲����غ󣬶�ȡ���ݾ��ǲ�׼ȷ�ġ�����Զ������Ϊ�ַ�����ȡ����������ˡ�
    // ��ʱ�ĵ�Ч�Ľ���취�ǣ���ʽ��Ϊ�ַ��������غ���ת��Ϊqint64
    //    p->data.push_back(QByteArray((const char*)&send_no, sizeof(qint64)));

    qDebug() << QStringLiteral("�첽�������󣬷���")<<service_name << QStringLiteral("��ţ�")<<send_no;

    // 4, data
    p->data.push_back(data);

    queue_locker_.lock();
    msgs_.enqueue(p);
    queue_locker_.unlock();


    // ����һ���첽������Ϣ
    // ��socket���Ա��Ǵ����߳�ֱ�ӷ��ʣ���Ϊ�����ˡ�
    AwakeZmq(AWAKE_REASON_POSTMSG);

    return send_no;
}

qint64 KAsyncReqThreadPrivate::PostRequest(
    const QString& connection_name,
    const QString& service_name,
    const QByteArray& data)
{
    return PostRequest(nullptr, nullptr, connection_name, service_name, data);
}

bool KAsyncReqThreadPrivate::run()
{
    // qRegisterMetaType<ReciveMessage>("ReciveMessageType");

    // ����ѭ����socket��������ջ�����Ϣ��ʹ��inprocЭ��
    nzmqt::KMySocket* sock_awake_server = nullptr;

    ctx_ = new nzmqt::KMyZmqContext();

    // �������Ѽ������ӣ�ʹ��inprocЭ��
    sock_awake_server = AddConnection_p(AWAKE_CONNECTION_SERVERNAME,
        nzmqt::ZMQSocket::TYP_REP, AWAKE_CONNECTION_ADDR, true);

    if (nullptr == sock_awake_server)
    {
        Q_ASSERT(false);
        qCritical() << QStringLiteral("��������socketʧ�ܣ�");

        // throw nzmqt::ZMQException();
        return false;
    }

    // ����������
    awaker_ = new AWaker(ctx_, AWAKE_CONNECTION_ADDR);

// #ifdef _DEBUG
//     // ���ڲ��Ե�����
//     AddConnection_p("test_conn", nzmqt::ZMQSocket::TYP_DEALER, "tcp://127.0.0.1:5555", false);
// #endif

    qDebug() << QStringLiteral("�����첽��Ϣѭ����");

    while (!stoped_)
    {
        try
        {
            qDebug() << QStringLiteral("����zmq epollѭ�������ڼ�����������....");
            Q_ASSERT(nullptr != ctx_);
            if (ctx_->poll(&recvs_, sock_awake_server))
            {
                // ������Ϣ��ʹ��ͬ��ģʽ��ȡ
                QList<QByteArray> m = sock_awake_server->receiveMessage();

                // ���յ���Ϣ��һ��Ҫ�ظ�һ����Ϣ��
                // FUCK!!! ����˵�õģ�����һֱ����/һֱ���յ��𣿣�����
                sock_awake_server->sendMessage("");

                // ������Ϣ
                if (m.size() != 2)
                {
                    Q_ASSERT(false);
                    continue;
                }

                if (QString(m[0]).compare(AWAKE_REASON_SHUTDOWN) == 0)
                {
                    // �˳���
                    break;
                }
                else if (QString(m[0]).compare(AWAKE_REASON_POSTMSG) == 0)
                {
                    // �����Ͷ��� 
                    HandleSendingQueue();
                }
                else if(QString(m[0]).compare(AWAKE_REASON_ADDCONNECTION) == 0)
                {
                    // �������
                    RJsonDoc cfg(m[1], false);
                    if (!cfg.HasParseError())
                    {
                        AddConnection_p(cfg["name"].GetString(),
                            (nzmqt::ZMQSocket::Type)cfg["type"].GetInt(),
                            cfg["addr"].GetString(),
                            cfg["listen"].GetBool());
                    }
                }
                else if (QString(m[0]).compare(AWAKE_REASON_ADDHEADSUBSCRIBER) == 0)
                {
                    // �������
                    RJsonDoc cfg(m[1], false);
                    if (!cfg.HasParseError())
                    {
                        recvs_.SubscribeHead(
#ifdef _WIN64
                            (QObject*)cfg["object"].GetUint64(),
#else
                            (QObject*)cfg["object"].GetInt(),
#endif
                            cfg["slot"].GetString(),
                            cfg["head"].GetInt());
                    }
                }
                else if (QString(m[0]).compare(AWAKE_REASON_UNREGISTEROBJECT) == 0)
                {
                    RJsonDoc cfg(m[1], false);
                    if (!cfg.HasParseError())
                    {
                        recvs_.UnregisterObject(
#ifdef _WIN64
                            (QObject*)cfg["object"].GetUint64()
#else
                            (QObject*)cfg["object"].GetInt()
#endif
                            );
                    }
                }
            }
            qDebug() << QStringLiteral("�˳�zmq epollѭ��....");
        }
        catch (const nzmqt::ZMQException& ex)
        {
            qWarning("Exception during poll: %s", ex.what());
        }
    }


    // ���Ȼ�����ɾ��
    delete awaker_;
    awaker_ = nullptr;

    // delete
    ctx_->stop();

    for (nzmqt::KMySocket* s : sockets_.values())
    {
        s->close();

        delete s;
    }

    delete ctx_;
    ctx_ = nullptr;

    return false;
}

void KAsyncReqThreadPrivate::HandleSendingQueue()
{
    if (!queue_locker_.tryLock())
    {
        return;
    }

    ON_SCOPE_EXIT([=](){
        queue_locker_.unlock();
    });

    // һ���Է��Ͷ��������е�����
    while (!stoped_ && !msgs_.isEmpty())
    {
        _SendingItem* p = msgs_.dequeue();
        auto* sock = FindSocket(p->connection_name);
        if (nullptr == sock)
        {
            qCritical() << QStringLiteral("û���ҵ���������");
        }
        else
        {
#ifdef _DEBUG
            qDebug() << QStringLiteral("�첽���ͣ�")
                << QStringLiteral("��������") << p->service_name
                << QStringLiteral("��ţ�") << p->no_
                << QStringLiteral("���ݣ�") << p->data
                << QStringLiteral("���󱻴���");
            qDebug() << "---> Still has:" << msgs_.size();
#endif // DEBUG

            // ����������
            sock->sendMessage(p->data, nzmqt::ZMQSocket::SND_NOBLOCK);
        }
    }
}

void KAsyncReqThreadPrivate::stop()
{
    stoped_ = true;

    if (nullptr != awaker_)
    {
        // ͬ��������ͨ��
        AwakeZmq(AWAKE_REASON_SHUTDOWN, "1" , false);
    }
}

nzmqt::KMySocket* KAsyncReqThreadPrivate::AddConnection_p(const QString&connection_name, 
    nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen)
{
    auto *sock = FindSocket(connection_name);
    if (nullptr != sock)
    {
        // Q_ASSERT(false);
        qWarning() << QStringLiteral("��������ע�ᣡ");
        // return nullptr;
        // ��ԭ����ɾ����
        // FIXME: socket��Ҫ�ֶ�delete��
        sock->close();
    }

    sock = (nzmqt::KMySocket*)ctx_->createSocket(typ);
    Q_ASSERT(nullptr != sock);
    if (nullptr != sock)
    {
        if (listen)
        {
            sock->bindTo(addr_port);
        }
        else
        {
            sock->connectTo(addr_port);
        }

        sockets_.insert(connection_name.toLower(), sock);
    }

    return sock;
}

void KAsyncReqThreadPrivate::AddConnection(const QString&connection_name,
    nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen)
{
    KConfig cfg;
    cfg.AddMember("name", connection_name);
    cfg.AddMember("type", typ);
    cfg.AddMember("addr", addr_port);
    cfg.AddMember("listen", listen);

    AwakeZmq(AWAKE_REASON_ADDCONNECTION, cfg.GetConfigString().toUtf8());
}

void KAsyncReqThreadPrivate::AwakeZmq(const char* reason, QByteArray data, bool use_async)
{
    QList<QByteArray> msg;
    // DEALER �� DEALER ���ӵ�ʱ�򣬵�һ֡Ϊ����Ϊ��֡��
    // ����poll���Ա����ѣ����ǲ����ܽ��յ���Ϣ
    // ������Ϣ����false������ֵ��EAGAIN
    msg.push_back("");  
    msg.push_back(reason);
    msg.push_back(data);

    if (nullptr != awaker_)
    {
        awaker_->AwakeMessage(msg, use_async);
    }
}

void KAsyncReqThreadPrivate::UnregisterRequestForObject(QObject* obj)
{
    KConfig cfg;
#ifdef _WIN64
    cfg.AddMember("object", (qulonglong)obj);
#else
    cfg.AddMember("object", (long)obj);
#endif

    AwakeZmq(AWAKE_REASON_UNREGISTEROBJECT, cfg.GetConfigString().toUtf8());
}

void KAsyncReqThreadPrivate::SubscribeHead(int head_no, QObject* obj, const char* slot)
{
    KConfig cfg;
    cfg.AddMember("head", head_no);
#ifdef _WIN64
    cfg.AddMember("object", (qulonglong)obj);
#else
    cfg.AddMember("object", (long)obj);
#endif
    cfg.AddMember("slot", slot);

    AwakeZmq(AWAKE_REASON_ADDHEADSUBSCRIBER, cfg.GetConfigString().toUtf8());
}
