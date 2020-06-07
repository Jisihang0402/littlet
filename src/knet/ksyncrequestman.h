#ifndef krequestman_h__
#define krequestman_h__

#include <QtCore/QList>
#include <QtCore/QString>
#include "deps/nzmqt/nzmqt.hpp"

#pragma comment(lib, "libzmq.lib")

class SyncReq : public QObject
{
    Q_OBJECT;

public:
    SyncReq()
    {
        connected_ = false;
        ip_ = "tcp://127.0.0.1:5555";
        sock_ = nullptr;
        ctx_ = new nzmqt::SocketNotifierZMQContext;
    }

    ~SyncReq()
    {
        // ��Ϊ�˶�����ȫ�ֵģ����ڽ��̼���������ʱ�����˳�main�������Ż�����
        // �ڴ˴�����Shutdown�����Ļ��������zmq�ڲ������쳣��
        // ����Ӧ��������֮ǰӦ�õ��� ShutDown()
        Q_ASSERT(nullptr == sock_);
        Q_ASSERT(nullptr == ctx_);
    }

    void Shutdown()
    {
        if (nullptr != sock_)
        {
            sock_->close();

            delete sock_;
            sock_ = nullptr;
        }

        if (nullptr != ctx_)
        {
            delete ctx_;
            ctx_ = nullptr;
        }
    }

    void conn()
    {
        if (!connected_)
        {
            
            if (sock_ == nullptr)
            {
                // sock_ = ctx_.createSocket(nzmqt::ZMQSocket::TYP_REQ);
                sock_ = ctx_->createSocket(nzmqt::ZMQSocket::TYP_DEALER);
                sock_->setIdentity("cms_test");
            }
			sock_->connectTo(ip_);
			//sock_->connectTo("tcp://192.168.205.202:5555");

            connected_ = true;
        }
    }

    void SetServer(const QString& ip)
    {
        ip_ = ip;

        if (sock_ == nullptr)
        {
            sock_ = ctx_->createSocket(nzmqt::ZMQSocket::TYP_DEALER);
            sock_->setIdentity("cms_test");
        }
        
        if (connected_)
        {
            sock_->close();
        }

        sock_->connectTo(ip);
        connected_ = true;
    }

    bool SendRequest(const QString& name, const QList<QString>& data,
        QList<QString>& out_data)
    {
        using namespace nzmqt;

        // ��4֡��Ϣ
        // 1, 0byte �մ�
        // 2, ������
        // 3, �����߱�ʶ��
        // 4, ��Ϣ����
        static const QByteArray NULL_BYTEARRAY;

        QList<QByteArray> msg;
        // 1,
        msg.push_back(NULL_BYTEARRAY);
        // 2, 
        msg.push_back(name.toUtf8());
        // 3,
        msg.push_back("TEST_ID");
        // 4,
        for each(QString v in data)
        {
            QByteArray b = v.toUtf8();
//                 v.toUtf8();
//             b = v.toLatin1();
//             b = v.toLocal8Bit();
            msg.push_back(b);
        }
        
        if (sock_->sendMessage(msg, 0))
        {
            const char* str = zmq_strerror(zmq_errno());
            
            QList<QByteArray> parts;

            ZMQMessage msg;
            while (sock_->receiveMessage(&msg, 0))
            {
                parts += msg.toByteArray();
                msg.rebuild();

                if (!sock_->hasMoreMessageParts())
                    break;
            }

            for each ( auto v in parts)
            {
                out_data.push_back(v);
            }

            // ǰ��֡�����Ǳ�ʶ����
            out_data.removeFirst();
            out_data.removeFirst();
            out_data.removeFirst();

            return true;
        }
        return false;
    }

private:
    nzmqt::SocketNotifierZMQContext* ctx_;
    nzmqt::ZMQSocket*  sock_;
    bool    connected_;
    QString ip_;
};


#endif // krequestman_h__
