#pragma once

#include <QObject>
#include <QThread>

#include "deps/qxtglobal.h"
#include "deps/nzmqt/nzmqt.hpp"

/**
 *  �첽�������շ�		
 *
 *
**/
class KAsyncReqThread : public QThread
{
    QXT_DECLARE_PRIVATE(KAsyncReqThread);

public:
    KAsyncReqThread();
    ~KAsyncReqThread();

    /**
     *	@param
     *		-[in]
     *          json_config     ��������
    **/
    bool Startup(const QString& json_config);

    void Shutdown();

    /**
     *	�첽������������	
     *
     *	@param
     *		-[in]
     *      sender_obj  �����߶��󣬱���ΪQObject�������࣬��ͨ���źŸ�֪��������
     *      connection_name     ʹ���ĸ�socket���ⷢ��
     *      service_name        �����������÷������ϵ�ʲô����
     *      data                ��Ҫ���͵�����
    **/
    qint64 PostRequest(QObject* sender_obj, const char* slot, 
        const QString& connection_name, const QString& service_name, 
        const QByteArray& data);
  
    void AddConnection(const QString&connection_name,
        nzmqt::ZMQSocket::Type typ, const char* addr_port, bool listen);

    void SubscribeHead(int head_no, QObject* obj, const char* slot);

    void UnregisterRequestForObject(QObject* obj);

protected:
    virtual void run();
};

