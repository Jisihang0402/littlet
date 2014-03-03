#pragma once

#include "BaseType.h"
#include "QBuffer.h"
#include "ui/QWindow.h"
#include "inet/http/http_request_manager.h"
#include "AppHelper.h"
#include "ui/QUIMgr.h"
#include "file/rjson/inc.h"
#include "UserInfo.h"

#define UPDATION_TEMP_FILE L"__temp_update.zip"

inline QString __UpdationTempFile()
{
    return quibase::GetModulePath() + UPDATION_TEMP_FILE;
}

#define BASEDATA_CACHE_FILE L"__basedata.dat"

inline QString __BasedataCacheFile()
{
    return quibase::GetModulePath() + BASEDATA_CACHE_FILE;
}


/**
*	���ݱ�����ͼƬ��	
*
*	@param
*		-[in]
*          bufEncrypt      ͼƬ���ݣ��Ѽ���
*		-[out]
*          bufData         ���ܺ������
**/
BOOL DecryptPictureData(__in QBuffer& bufEncrypt, __out QBuffer& bufData);

struct _Url 
{
    QString     url_;
    QString     domain_;
};

#define UIBASE_MAILPSWD_PATH L"_qiuinfo.json"

// ������������ + ��Ϣ���ģ����صĴ��ڣ�
class QUIConnectCenter : 
    public FCHttpRequestManager,
    public CWindowImpl<QUIConnectCenter>
{
    friend class QUserFeedbackWnd;

    enum ENUM_DOWNLOAD_TYPE
    {
        DOWNLOAD_TYPE_QUI = 0,  // QUIBase ��Ҫ   
        DOWNLOAD_TYPE_APP,  // Ӧ�ó�����Ҫ
        DOWNLOAD_TYPE_USER,     // �û�����
    };

    BEGIN_MSG_MAP_EX(QUIConnectCenter)
        MSG_WM_CLOSE(OnClose)
    END_MSG_MAP()

    SINGLETON_ON_DESTRUCTOR(QUIConnectCenter)
    {
        if (IsWindow())
        {
            DestroyWindow();

            m_hWnd = NULL;
        }
    }

protected:
    // ��UI.zip �� url.x �ļ���ȡ�����ַ����
    //  1���Զ����µ�ַ
    //  2���û����ݣ�BaseData����ַ
    //  3���û�ͳ�Ƶ�ַ
    class _QUrlExtract
    {
        typedef std::vector<_Url>   LstAddr;

    public:
        // ��ȡ
        BOOL Init();

        BOOL GetUpdationUrl(__inout _Url& adr)
        {
            return GetAddr(update_url_, adr);
        }

        BOOL GetUserUrl(__inout _Url& adr)
        {
            return GetAddr(user_url_, adr);
        }

        // �û�ͳ��ҳ��
        QString GetStaticsUrl()const
        {
            return stat_url_;
        }

    protected:
        BOOL GetAddr(__in LstAddr& lst, __inout _Url& adr);

    private:
        LstAddr     update_url_;    // �Զ����µĵ�ַ
        LstAddr     user_url_;    // �û���Ҫ������
        QString     stat_url_;  // �û�ͳ��
    }ui_url_;

protected:
    /**
    *  ����������������BaseData���ݰ�
    *
    *	��zip�а���������Ӧ����	
    *      1���û�������bug�ύ����Ҫ��client��web����
    *      2��������ݣ���ѡ��
    **/
    class _QBaseData : public QUIResource
    {
    public:
        // ����ԭʼ�ļ�������
        BOOL SetBaseData(__in QBuffer& bufEncrypt)
        {
            ReleaseAll(NULL);

            QBuffer buf;
            if (!DecryptPictureData(bufEncrypt, buf))
                return FALSE;

            return QUIResource::LoadFromZip(buf);
        }

        virtual void OnReleaseResource(__in const QString& szResName, 
            __in const QString& szResIndicator)
        {

        }

        BOOL LoadDataFile(__in const QString& sFile = __BasedataCacheFile())
        {
            QBuffer buf;
            if (buf.FileRead(sFile))
            {
                return SetBaseData(buf);
            }
            return FALSE;
        }

    }basedata_;

protected:
    // ���ڱ��� �������/bug�ύ ����Ҫ���ʼ������룬�����ַ��
    class _QiuInfo
    {
    public:
        typedef std::vector<QString> LstAddr;
        typedef std::vector<_CLIENT_MAIL> LstCM;

    public:
        /**
        *	���õ�ַ�ļ�������֮	
        *
        *	@return
        *	    -1      	��������
        *      0           ����������Ϊ��
        *      >0          ����������
        *	@param
        *		-[in]
        *          sFile       json ����
        *
        **/
        int SetAddrData(__in const QString& sJsonData) throw()
        {
            // �ļ���ʽ
            // {
            //     "server":
            //     {
            //         "web":
            //         [
            //             "http://www.baidu.com",
            //             "http://www.qiuchengw.com"
            //         ]
            //         ,
            //             "email":
            //         [
            //             "qiuchengw@qq.com",
            //             "qiuchengw@163.com"
            //         ]
            //     },
            //         "client":
            //     {
            //         "ziyue_yulu@163.com":
            //         {
            //             "pswd":"ZaiHangzhou88",
            //                 "host":"smtp.163.com",
            //                 "port":465,
            //                 "user":"ziyue_yulu"
            //         }
            //     }
            // }
            RJsonDocumentW d;
            d.Parse<0>(sJsonData);
            if (d.HasParseError())
                return FALSE;

            // servers
            RJsonValueW& srv = d[L"server"];
            if (srv.IsObject())
            {
                // web
                RJsonValueW& web = srv[L"web"];
                if (web.IsArray())
                {
                    webs_.clear();
                    for (auto i = web.Begin(); i != web.End(); ++i)
                    {
                        webs_.push_back((*i).GetString());
                    }
                }

                // mails
                RJsonValueW& mail = srv[L"email"];
                if (mail.IsArray())
                {
                    mails_.clear();
                    for (auto i = mail.Begin(); i != mail.End(); ++i)
                    {
                        mails_.push_back(i->GetString());
                    }
                }
            }

            // client
            RJsonValueW& cmail = d[L"client"];
            if (cmail.IsObject())
            {
                client_mail_.clear();
                for (auto i = cmail.MemberBegin(); i != cmail.MemberEnd(); ++i)
                {
                    RJsonValueW& v = i->value;
                    _CLIENT_MAIL cm;
                    cm.mail_ = i->name.GetString();
                    cm.host_ = v[L"host"].GetString();
                    cm.user_ = v[L"user"].GetString();
                    cm.pswd_ = v[L"pswd"].GetString();
                    cm.port_ = v[L"port"].GetInt();
                    client_mail_.push_back(cm);
                }
            }

            return mails_.size() + webs_.size() + client_mail_.size();
        }

        /**
        *	��ȡ����ʵ�mail��ַ	
        *
        *	@param
        *		-[inout]
        *          adr     �����Ϊ�գ����ڵ�ַ�в�������Ȼ�󷵻�������������Ǹ��Ǹ���ַ
        *                  ���Ϊ�գ���ȡmail�б�����ǰ����Ǹ���ַ
        **/
        inline BOOL GetMailAddr(__inout QString& adr)
        {
            return GetAddr(mails_, adr);
        }

        inline BOOL GetWebAddr(__inout QString& adr)
        {
            return GetAddr(webs_, adr);
        }

        /**
        *	��ȡ���ڷ����ʼ��ķ�����
        *
        *	@param
        *		-[inout]
        *          cm
        **/
        BOOL GetClientMail(__inout _CLIENT_MAIL& cm)
        {
            if (client_mail_.empty())
                return FALSE;

            if (cm.mail_.IsEmpty())
            {
                cm = client_mail_.front();
                return TRUE;
            }

            auto iEnd = client_mail_.end();
            for (auto i = client_mail_.begin(); i != iEnd; ++i)
            {
                if (cm.mail_.CompareNoCase((*i).mail_) == 0)
                {
                    if (++i != iEnd)
                    {
                        cm = *i;
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }

    protected:
        BOOL GetAddr(__in LstAddr& lst, __inout QString& adr)
        {
            if (lst.empty())
                return FALSE;

            if (adr.IsEmpty())
            {
                adr = lst.front();
                return TRUE;
            }

            auto iEnd = lst.end();
            for (auto i = lst.begin(); i != iEnd; ++i)
            {
                if (adr.CompareNoCase(*i) == 0)
                {
                    if (++i != iEnd)
                    {
                        adr = *i;
                        return TRUE;
                    }
                }
            }
            return FALSE;
        }

    protected:
        LstAddr     mails_;
        LstAddr     webs_;
        LstCM       client_mail_;
    }qiu_info_;

public:
    // �����ʼ�������ʹ��
    BOOL Init();

    _QiuInfo* GetQiuInfo()
    {
        return &qiu_info_;
    }

    inline BOOL IsInited()const
    {
        return IsWindow();
    }

    BOOL GetUpdationUrl(__inout _Url& adr)
    {
        return ui_url_.GetUpdationUrl(adr);
    }

    BOOL GetUserUrl(__inout _Url& adr)
    {
        return ui_url_.GetUserUrl(adr);
    }

    /*
    *	�û����������bug�ύ����Ҫ����������
    */
    BOOL LoadBaseData(__in const QString& sBaseDataFile);

    /*
    *	��Ӧ�ó������������ͳ��ҳ��һ��
    */
    BOOL VisitStatisticPage();

protected:
    // msgs
    void OnClose();

protected:
    //@{
    /// callback after connected to server and sent HTTP request.
    virtual void OnAfterRequestSend (FCHttpRequest& rTask) ;

    /**
    callback after request finish\n
    after this callback, request will be deleted.
    */
    virtual void OnAfterRequestFinish (FCHttpRequest& rTask) ;

    /**
    callback when request over its lifetime that user set in HTTP_REQUEST_HEADER::m_lifetime_limit\n
    after this callback, request will be deleted.
    */
    virtual void OnOverRequestLifetime (FCHttpRequest& rTask) ;
};


/** ����Ӧ�ó����ļ�
*	return:
*      �޷���
*	params:
*		-[in]
*          wParam      1���и��£� 0����ǰ�޸���
*          lParam      ��wParam==0������ҲΪ0 
*                        wParam==1������ΪLPQUI_USERMSGPARAM��
*  -----------------------------------------
*          LPQUI_USERMSGPARAM ���ֵΪ��
*                      wParam  1�����³ɹ�
*                              0������ʧ��
*                      lParam  1��baseData ����
*                              0��Ӧ�ó������
*                      sParam  �����صĸ����ļ�·��
*                      bFreeIt TRUE    ������Ӧ��ɾ���������
*		-[out]
*      �����
**/
DECLARE_USER_MESSAGE(QSOFT_MSG_UPDATEAPP);

class QAutoUpdater
{
    static VOID CALLBACK CheckUpdateCallback(
        __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);

    SINGLETON_ON_DESTRUCTOR(QAutoUpdater)
    {
        if (NULL != m_hTimer)
        {
            DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
        }
    }

public:
    QAutoUpdater(void);

    /** �����Զ����³���
    *	return:
    *      TRUE    �����ɹ�
    *	params:
    *		-[in]
    *          dwCheckPeriod   �����µ�Ƶ�ʣ���λʱ�䣨�֣�
    *                          ��С���Ƶ��Ϊ10����
    *          sURL            �����ַ
    *          szRefer         �ƽ������ �����ã�http://www.sina.com��
    *
    **/
    //BOOL Startup(__in _Url& urlUpdation, __in _Url& urlBaseData, __in DWORD dwCheckPeriod = 30);
    BOOL Startup(__in DWORD dwCheckPeriod = 30);

protected:
    /** �첽��ʽ���������.
    *      ����������ɹ����������󽫻������������ڷ���һ��QSOFT_MSG_UPDATEAPP��Ϣ
    *	return:
    *      TRUE    �����ɹ�
    **/
    BOOL CheckUpdate( );
    BOOL CheckBaseData( );

    enum ENUM_TASK_TYPE 
    {
        TASK_TYPE_UPDATION = 1,
        TASK_TYPE_BASEDATA = 2,
    };

    // �����̻߳ص�����
    struct _Download_Param 
    {
        _Download_Param (ENUM_TASK_TYPE t)
            :pData(nullptr), type_(t)
        {

        }

        QString sURL;	// in
        QString sRefer;	// in
        LPVOID	pData; // in
        QBuffer	bufDown;	// out ���ص�����

        inline BOOL IsUpdationTask()const
        {
            return TASK_TYPE_UPDATION == type_; 
        }

    private:
        ENUM_TASK_TYPE type_;   // ��������
    };
    /** �������ݵ��߳�
    *	params:
    *		-[in]
    *          param            _Download_Param* 
    **/
    static uint __stdcall thread_download(void* param);
private:
    //BOOL				m_bEnd;
    HANDLE              m_hTimer;   // �����µ�ʱ�䶨ʱ��
    //     QString             m_sUrl;
    //     QString             m_sRefer;   // �����ƽ������
    _Url        url_updation_;
    _Url        url_basedata_;
};

