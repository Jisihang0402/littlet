#pragma once

#include <winsock2.h>
#include <windows.h>

#include <vector>
#include <list>

#include "WorkQueue.h"

#include "deps/zmq.hpp"

#pragma comment(lib, "libzmq.lib")

typedef std::vector<zmq::message_t*> OneMsg;
struct  user_feed_param
{
    zmq::context_t* ctx;
    std::string     ip_port;  
};

struct UserMsg
{
    UserMsg()
    {
        handled = false;
    }

    OneMsg  msg;
    bool handled;
};

typedef std::list<UserMsg*> Msgs;

/**
 *	����������
 *      1�������û��������
 *      2�������Զ��ϴ���bug��todo��
 *
**/
class CMsgServer
{
public:
    CMsgServer(void);
    ~CMsgServer(void);

    // ��������
    int Startup();

protected:
    // ���ݴ����߳�
    static UINT_PTR __stdcall thread_handlemsg(void* param);

private:
    // ��������߳�
    zmq::context_t  ctx_;
};

