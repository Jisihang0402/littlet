#include "server.h"
#include <process.h>
#include <iostream>

#include "stdstring.h"
#include "db.h"

#define WT_NUM 10


//////////////////////////////////////////////////////////////////////////

class CUserFeedWorkItem : public WorkItemBase
{
public:
    CUserFeedWorkItem(user_feed_param* p)
    {
        run_ = false;
        sock_ = nullptr;
        param_ = p;
    }

    ~CUserFeedWorkItem()
    {
        if (nullptr != sock_)
        {
            delete sock_;
        }
    }

    virtual void   DoWork(void* pThreadContext);
    
    virtual void   Abort ();

    /**
     *	�����Ϣ������Ļ����Ӷ�����ɾ��	
     *
     *	@return
     *	    ��û�б��������Ϣ����	
     *
    **/
    int scan_usefeed();

    const Msgs* msgs()const
    {
        return &msgs_;
    }

private:
    Msgs msgs_;
    bool run_;
    zmq::socket_t*  sock_;
    user_feed_param* param_;   
};

typedef std::vector<CUserFeedWorkItem*> Workers;

//////////////////////////////////////////////////////////////////////////

CMsgServer::CMsgServer(void)
{
}

CMsgServer::~CMsgServer(void)
{
}

int CMsgServer::Startup()
{
    // ����10���������߳�
    zmq::context_t ctx;
    user_feed_param ufp;
    ufp.ctx = &ctx;
    ufp.ip_port = "inproc://*:5230";

    // �������߳�
    CWorkQueue  work_queue;
    work_queue.Create(WT_NUM);

    // ֱ��ȫ�������ɣ�
    Workers workers;
    for (int i = 0; i < WT_NUM; ++i)
    {
        CUserFeedWorkItem* a_worker = new CUserFeedWorkItem(&ufp);
        if (work_queue.InsertWorkItem(a_worker))
        {
            workers.push_back(a_worker);
        }
        else
        {
            delete a_worker;
        }
    }

    // ʹ��dealer���տͻ�����Ϣ
    zmq::socket_t front_end(ctx, ZMQ_ROUTER);
    front_end.bind("tcp://*:5231");  // ���صĶ˿�
    
    zmq::socket_t back_end(ctx, ZMQ_DEALER);
    back_end.bind(ufp.ip_port.c_str());  // ���صĶ˿�

    // �������ݵ��߳�
    HANDLE h_feed = (HANDLE)_beginthreadex(nullptr, 0, 
        &CMsgServer::thread_handlemsg, &workers, 0, nullptr);
    CloseHandle(h_feed);

    // ��������
    zmq::proxy(front_end, back_end, nullptr);

    return 0;
}

CStdString GetModulePath()
{
    TCHAR path[1024];
    int nSize = ::GetModuleFileName(NULL,path,1024);
    path[nSize] = _T('\0');
    CStdString sRet(path);
    sRet.Replace(_T('\\'),_T('/'));
    int idx = sRet.ReverseFind(_T('/'));
    sRet = sRet.Left(idx+1);
    return sRet;
}

UINT_PTR __stdcall CMsgServer::thread_handlemsg( void* param )
{
    // �������ݿ�
    if (!db::GetInstance()->Open(GetModulePath() + L"littlet.db"))
    {
        return -1;
    }

    Workers* ws = reinterpret_cast<Workers*>(param);
    // �ȴ���Ϣ
    while (true)
    {
        // ɨ�����й������߳����յ�������
        for (auto i_worker = ws->begin(); i_worker != ws->end(); ++i_worker)
        {
            // ����
            const Msgs* ms = (*i_worker)->msgs();
            for (auto i_msg = ms->begin(); i_msg != ms->end(); ++i_msg)
            {
                const UserMsg *a_msg = *i_msg;
                if (a_msg->handled)
                    continue;   // �Ѿ��������

                // û�����
                const OneMsg& m = a_msg->msg;
                // ��3֡��
                //  1,������
                //  2,MAC ��ַ
                //  3,����
                if (m.size() < 3)
                {
                     const_cast<UserMsg*>(a_msg)->handled = true;
                     continue;
                }

                // ȫ��wchar_t���͵��ַ���
                CStdString svr_name((const wchar_t*)(m[0]->data()), m[0]->size() / 2);
                CStdString mac_addr((const wchar_t*)(m[1]->data()), m[1]->size() / 2);
                CStdString content((const wchar_t*)(m[2]->data()), m[2]->size() / 2);

                std::wcout<<L"user ["<<mac_addr.c_str()<<L"] activity: ["<<svr_name.c_str()<<"]"<<std::endl;

                // ���ݷ���������Ӧ�Ĳ���
                if (svr_name.CompareNoCase(L"feed") == 0)
                {
                    // д�����ݿ�
                    db::GetInstance()->UserFeedback(mac_addr, content, L"");
                }
                else if (svr_name.CompareNoCase(L"login") == 0)
                {
                    // д�����ݿ�
                    db::GetInstance()->UserLogin(mac_addr);
                }
                else if (svr_name.CompareNoCase(L"logout") == 0)
                {
                    // д�����ݿ�
                    db::GetInstance()->UserLogout(mac_addr);
                }
                else if (svr_name.CompareNoCase(L"require_feed_qcw") == 0)
                {
                    // д�����ݿ�
                    db::GetInstance()->UserLogout(mac_addr);
                }

                // �������
                const_cast<UserMsg*>(a_msg)->handled = true;
            }
        }

        Sleep(10);
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
void CUserFeedWorkItem::DoWork( void* thread_context )
{
    // param_ = *reinterpret_cast<user_feed_param*>(thread_context);
    assert(nullptr != param_);

    // �������߳�
    sock_ = new zmq::socket_t(*(param_->ctx), ZMQ_REP);
    sock_->connect(param_->ip_port.c_str());

    run_ = true;

    // �ȴ���Ϣ�ĵ���
    int has_more;
    while (run_)
    {
        scan_usefeed();

        UserMsg *uf = new UserMsg;
        OneMsg& msg = uf->msg;

        bool bok = false;
        while (run_)
        {
            zmq::message_t* msg_frame = new zmq::message_t;

            std::cout<<"LitteT server tool thread: begin recv ...."<<std::endl;

            // ����ʽ������Ϣ
            if (!sock_->recv(msg_frame))
            {
                delete msg_frame;
                break;
            }
#ifdef _DEBUG
            std::cout<<"msg recived."<<std::endl;
#endif
            // ��¼һ֡��Ϣ
            msg.push_back(msg_frame);

            // �Ƿ������Ϣ��
            size_t size_len = sizeof(has_more);
            sock_->getsockopt(ZMQ_RCVMORE, &has_more, &size_len);
            if (!has_more)
            {
                bok = true;
                break;
            }
        }
        
        if (bok)
        {
            // һ����Ϣ�������, ���Է������ݶ�����
            msgs_.push_back(uf);

            // ����һ����Ϣ
            const char* buf = "thanks!";
            sock_->send(buf, strlen(buf));
        }
        else
        {
            delete uf;
        }
    }
}

void CUserFeedWorkItem::Abort()
{
    run_ = false;

    if (nullptr != sock_)
    {
        sock_->disconnect(param_->ip_port.c_str());
    }
}

int CUserFeedWorkItem::scan_usefeed()
{
    auto i_e = msgs_.end();
    for (auto i_msg = msgs_.begin(); i_msg != i_e; )
    {
        if ((*i_msg)->handled)
        {
            OneMsg& m = (*i_msg)->msg;
            for (auto i_frame = m.begin(); i_frame != m.end(); ++i_frame)
            {
                delete *i_frame;
            }
            delete *i_msg;

            i_msg = msgs_.erase(i_msg);
        }
        else
        {
            ++i_msg;
        }
    }
    return msgs_.size();
}
