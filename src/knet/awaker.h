#ifndef awaker_h__
#define awaker_h__

#include <QHash>
#include <QMutex>
#include "deps/nzmqt/nzmqt.hpp"

class AWaker
{
    Q_DISABLE_COPY(AWaker);

    friend class KAsyncReqThreadPrivate;

private:
    AWaker(nzmqt::KMyZmqContext* ctx, const char* awake_addr)
        :ctx_(ctx), awake_addr_(awake_addr)
    {
        // 1���߳��㹻��
        // ctx_ = ctx; new nzmqt::PollingZMQContext(nullptr, 1);
        Q_ASSERT(nullptr != ctx);
        Q_ASSERT(nullptr != awake_addr);
    }

    ~AWaker()
    {
        QMutexLocker locker(&event_locker_);

        for (auto *p : awake_socks_.values())
        {
            delete p;
        }
        // delete ctx_;
    }

    /**
     *	�򱻻����߷�����Ϣ	
     *
     *	@return
     *		true    ���ͻ��ѳɹ���
     *	@param
     *		-[in]
     *      msg     ������Ϣ
     *      use_async   false ����ʹ��ͬ��ģʽ���ͻ�����Ϣ
    **/
    bool AwakeMessage(QList<QByteArray>& msg, bool use_async)
    {
        event_locker_.lock();
        nzmqt::ZMQSocket* sock = socket(QThread::currentThreadId());
        event_locker_.unlock();

        Q_ASSERT(nullptr != sock);
        if (nullptr != sock)
        {
            return  sock->sendMessage(msg,
                (nzmqt::ZMQSocket::SendFlags)(use_async ? nzmqt::ZMQSocket::SND_NOBLOCK : 0));
        }
        return false;
    }

private:
    // �����ǰû���򴴽�
    nzmqt::ZMQSocket* socket(Qt::HANDLE thread_id)
    {
        auto * p = awake_socks_.value(thread_id);
        if (nullptr == p)
        {
            // ����ʹ���·�ʽ��������Ϊ�ڲ��߼��ǣ�
            //      1,����socket��
            //      2,ע��socket  ����ע���ʱ������ˣ����ܻ������������
            // p = ctx_->createSocket(nzmqt::ZMQSocket::TYP_DEALER, nullptr);

            p = new nzmqt::KMySocket(ctx_, nzmqt::ZMQSocket::TYP_DEALER);

            // ����
            awake_socks_.insert(thread_id, p);

            // ������ȥ
            p->connectTo(awake_addr_);
        }
        return p;
    }

private:
    // ÿ���߳����ҽ���һ������socket
    QHash<Qt::HANDLE, nzmqt::ZMQSocket*> awake_socks_;

    // ���еĻ���socket�������context�ϴ���
    nzmqt::KMyZmqContext* ctx_ = nullptr;

    // ��Ҫ�����ѵĵ�ַ
    QString     awake_addr_;

    // �¼���
    QMutex      event_locker_;
};

#endif // awaker_h__
