#pragma once

#include "BaseType.h"
#include <atlpath.h>
#include "file/rjson/inc.h"
#include "crypt/QDataEnDe.h"
#include "AppHelper.h"

namespace littlet
{
    inline BOOL IsFileExist(__in LPCWSTR szFile)    
    {
        return ATL::ATLPath::FileExists(szFile);
    }
}

struct _CLIENT_MAIL 
{
    QString     mail_;
    QString     user_;
    QString     pswd_;  // ���붼�Ǿ������ܵ�
    QString     host_;  // ������  �磺 smtp.163.com
    short       port_;  // �˿�
};

struct _USER_INFO 
{
    QString	name_;
    QString	pswd_;
    _CLIENT_MAIL cm_;
};

class _UserData
{
public:
    _UserData()
    {
        loaded_ = FALSE;
    }

    // ��һ������Ӧ�ó���ʹ���û����������ʼ��user��data
    BOOL FirstRun(__in LPCWSTR szUser, __in LPCWSTR szPswd)
    {
        if (littlet::IsFileExist(_ProfilePath()))
        {
            ASSERT(FALSE);
            return FALSE;
        }

        // ������ʼ�����û�����
        if (LoadEmptyWith(szUser, szPswd))
        {
            return SaveUserData(_ProfilePath());
        }
        return FALSE;
    }

    // ��������
    BOOL LoadProfile(__in LPCWSTR szUserName)
    {
        if (IsLoaded(szUserName))
            return TRUE;

        user_name_ = szUserName;
        user_name_.Trim();

        // ��ȡ����
        QBuffer buf;
        if (!buf.FileRead(_ProfilePath()))
            return FALSE;

        // ʹ���ṩ���û�����������
        QDataEnDe en;
        en.SetCodeData((LPCBYTE)(LPCWSTR)user_name_, 
            user_name_.GetLength() * sizeof(QString::value_type));
        if (!en.DecryptData(buf))
            return FALSE;
     
        // ��������
        doc_.Parse<0>((LPCWSTR)buf.GetBuffer());
        if (doc_.HasParseError())
            return FALSE;

//         {
//             "login":
//             {
//                 "name":"user_name",
//                 "pswd":"user_password"
//             },
//             "mail":
//             {
//                 "mail":"user@163.com",
//                 "user":"user",
//                 "pswd":"pswd",
//                 "port":465,
//                 "host":"smtp.163.com"
//             }
//         }
        // ������user_info_ / name��pswd
        {
            RJsonValueW& v = doc_[L"login"];
            user_info_.name_ = v[L"name"].GetString();
            if (user_info_.name_.Compare(user_name_) != 0)
                return FALSE;
            user_info_.pswd_ = v[L"pswd"].GetString();
        }

        // ������user_info_ / mail
        {
            RJsonValueW& v = doc_[L"mail"];
            if (v.IsObject())
            {
                _CLIENT_MAIL& c = user_info_.cm_;
                c.mail_ = v[L"mail"].GetString();
                c.user_ = v[L"user"].GetString();
                c.pswd_ = v[L"pswd"].GetString();
                c.port_ = v[L"port"].GetInt();
                c.host_ = v[L"host"].GetString();
            }
        }

        loaded_ = TRUE;

        return TRUE;
    }

    // ��������
    BOOL GetPassword( __out QString& sPswd)
    {
        if (IsLoaded(user_name_))
        {
            sPswd = user_info_.pswd_;

            return TRUE;
        }
        return FALSE;
    }

    // ��������
    BOOL SetPassword(const QString& sPswd)
    {
        if (IsLoaded(user_name_))
            return FALSE;

        doc_[L"login"][L"pswd"] = sPswd;
        
        return SaveUserData(_ProfilePath());
    }

    // �����ʼ���Ϣ
    BOOL GetClientMail(__out _CLIENT_MAIL& c)
    {
        if (IsLoaded(user_name_))
        {
            c = user_info_.cm_;
            return TRUE;
        }
        return FALSE;
    }

    _USER_INFO* GetUserInfo()
    {
        return &user_info_;
    }

    BOOL SetClientMail(__in const _CLIENT_MAIL& c)
    {
        if (!IsLoaded(user_name_))
        {
            ASSERT(FALSE);
            return FALSE;
        }

        user_info_.cm_ = c;

        RJsonValueW& v = doc_[L"mail"];
        v[L"mail"] = c.mail_;
        v[L"user"] = c.user_;
        v[L"pswd"] = c.pswd_;
        v[L"port"] = c.port_;
        v[L"host"] = c.host_;

        return SaveUserData(_ProfilePath());
    }

protected:
    QString _ProfilePath()
    {
        return quibase::GetModulePath() + L"user.dat";
    }

    BOOL IsLoaded(__in LPCWSTR szUser)const
    {
        return loaded_ && !user_name_.Compare(szUser);
    }

    // ����һ���յ�user.dat�ļ�
    BOOL LoadEmptyWith(__in LPCWSTR szUser, __in LPCWSTR szPswd)
    {
        user_name_ = szUser;

        doc_.SetObject();
        auto& alloctor = doc_.GetAllocator();

        // RJsonValueW rt(kObjectType); // root
        // login
        RJsonValueW login(rapidjson::kObjectType);
        login.AddMember(L"name", szUser, alloctor)
            .AddMember(L"pswd", szPswd, alloctor);
        doc_.AddMember(L"login", login, alloctor);
        // mail
        RJsonValueW mail(rapidjson::kObjectType);
        mail.AddMember(L"mail",L"", alloctor)
            .AddMember(L"user",L"",alloctor)
            .AddMember(L"pswd", L"", alloctor)
            .AddMember(L"port", 465, alloctor)
            .AddMember(L"host", L"", alloctor);
        doc_.AddMember(L"mail", mail, alloctor);

        loaded_ = TRUE;

        return TRUE;
    }

    BOOL SaveUserData(__in LPCWSTR szFile)
    {
        using namespace rapidjson;

        if (!user_name_.Trim().IsEmpty() && IsLoaded(user_name_))
        {
            RJsonStringBufferW sbuf;
            PrettyWriter<RJsonStringBufferW, UTF8_w > wrt(sbuf);
            wrt.SetIndent(L' ', 4); // 4���ո�
            doc_.Accept(wrt);

            QBuffer buf;
            buf.Write(sbuf.GetString());

            // ��������
            QDataEnDe en;
            en.SetCodeData((LPCBYTE)(LPCWSTR)user_name_, 
                user_name_.GetLength() * sizeof(QString::value_type));
            en.EncryptData(buf);
            
            // д�������ļ�
            return buf.FileWrite(szFile);
        }
        return FALSE;
    }

private:
    RJsonDocumentW  doc_;
    BOOL            loaded_;
    _USER_INFO     user_info_;
    QString        user_name_;
};

namespace littlet
{
    _USER_INFO* YuluGetUserInfo();

    _UserData* YuluGetUserData();
}

