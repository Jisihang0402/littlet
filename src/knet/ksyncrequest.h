#ifndef KSYNCREQUEST_H
#define KSYNCREQUEST_H


#include <QtCore/qglobal.h>

#ifdef KSYNCREQUEST_LIB
# define KSYNCREQUEST_EXPORT Q_DECL_EXPORT
#else
# define KSYNCREQUEST_EXPORT Q_DECL_IMPORT
#endif

class QObject;

typedef QList<QByteArray> ReciveMessage;

extern "C"
{

    /**
     *	���������������	
     *
    **/
    KSYNCREQUEST_EXPORT bool KStartNetRequestService();

    /**
    *	ͬ����������
    *
    *	@return
    *		true    ���ͳɹ�
    *
    *	@param
    *		-[in]
    *          name        ����������ķ�����
    *          data        ����
    *          out_data    ��������ķ�������
    **/
    KSYNCREQUEST_EXPORT bool KSyncSendRequest(const QString& name, const QList<QString>& data,
        QList<QString>& out_data);

    KSYNCREQUEST_EXPORT bool KSyncSendRequestA(const QString& name, const QString& data,
        QList<QString>& out_data);

    /**
    *	�첽��������
    *
    *	@return
    *		true    ���ͳɹ�
    *
    *	@param
    *		-[in]
    *          obj         ������
    *          slot        ��Ϣ���غ�Ļص���
    *          connection_name ��������ʹ���ĸ�����
    *          name        ����������ķ�����
    *          data        ����
    **/
    KSYNCREQUEST_EXPORT qint64 KAsyncPostRequest(QObject* obj, const char* slot,
        const QString& connection_name,const QString& service_name, const QByteArray& data);

    /**
    *	�첽��������
    *       �����������ݣ�����֪�����������ص����ݵİ�ͷ�ţ�
    *       ��ʹ�� KSubscribeHead ���������Ϣ
    **/
    KSYNCREQUEST_EXPORT qint64 KAsyncPostRequest2(const QString& connection_name, 
        const QString& service_name, const QByteArray& data);

    /**
    *	�������ٵ�ʱ��Ӧ�õ����������
    **/
    KSYNCREQUEST_EXPORT void KUnregisterRequestForObject(QObject* obj);

    /**
    *	���һ������
    *
    *	@param
    *		-[in]
    *          connection_name     �������Ʋ����ظ��������ִ�Сд
    *          addr_port           ��ַ�Ͷ˿ڣ�
    *                              �磺inproc://this_addr
    *                              .... tcp://1.2.3.4:555
    *          listen              true �󶨵����أ� false ���ӵ�Զ��
    **/
    KSYNCREQUEST_EXPORT bool KAsyncAddConnection(const QString&connection_name,
        const char* addr_port, bool listen);

    /**
     *	������Ϣͷ�����ж���	
     *
     *      json_msg["head"] == head_no ����Ϣ���ᱻ���͵����������
     *		
     *	@param
     *		-[in]
     *          head_no     ��ͷ�ţ�
     *          obj
     *          slot
    **/
    KSYNCREQUEST_EXPORT void KSubscribeHead(int head_no,
        QObject* obj, const char* slot);

    /**
     *  ����ip���ڷ���Ϣ֮ǰ���е��á�
     *
     *	@param
     *		-[in]
     *          ip_and_port     ip �� �˿ڣ����磺
     *                          tcp://192.168.205.202:5555
     *
    **/
    KSYNCREQUEST_EXPORT void KSetServerIP(const QString& ip_and_port);
    
    KSYNCREQUEST_EXPORT void KShutdownRequest();
}

#endif // KSYNCREQUEST_H
