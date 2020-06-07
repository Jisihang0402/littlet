#ifndef kconfig_h__
#define kconfig_h__

#include "deps/rjson/inc.h"

#ifdef QT_DLL

#pragma message("compile kconfig.h with Qt")

#include <QFile>
#include "misc.h"
typedef QString String;
#define _utf8_str(val) StrValue(val.toUtf8().constData(), cfg_->GetAlloctor())
#define _utf8_p(val) StrValue(val, cfg_->GetAlloctor())

#else   // c++ pure

#pragma message("compile kconfig.h with pure c++")

// ʹ��std::string
#ifndef RAPIDJSON_HAS_STDSTRING
#   define RAPIDJSON_HAS_STDSTRING 1
#endif

#include <vector>
#include <string>
typedef std::string String;
#define _utf8_str(val) StrValue(val.c_str(), cfg_->GetAlloctor())
#define _utf8_p(val) StrValue(val, cfg_->GetAlloctor())

#endif  // QT_DLL


class IConfig
{
public:
    virtual String GetConfig() = 0;
};

typedef rapidjson::Document RDoc;

class KConfigValue : public IConfig
{
    friend class KConfig;

    typedef RDoc::Ch MyCh;
    // typedef rapidjson::Value::StringRefType StrType;
    typedef rapidjson::Value StrValue;

    KConfigValue(const KConfigValue &) = delete;
    KConfigValue &operator=(const KConfigValue &) = delete;

protected:
    KConfigValue(rapidjson::Type typ, KConfig* c)
    {
        assert(nullptr != c);

        jv_ = new rapidjson::Value(typ);
        cfg_ = c;
    }

    KConfigValue()
    {
        jv_ = nullptr;
        cfg_ = nullptr;
    }

    virtual ~KConfigValue()
    {
        if (nullptr != jv_)
        {
            delete jv_;
        }

        for each(auto *p in values_)
        {
            delete p;
        }

//         for each(auto *p in strs_)
//         {
//             delete p;
//         }
    }

public:
    rapidjson::Value* jval()
    {
        return jv_;
    }

    // �������͵�ֵʹ��
    inline KConfigValue& PushBack(KConfigValue* v);

    virtual String GetConfig()
    {
        String s_conf;
        if (rapidjson::SaveToString(jv_, s_conf, true, 2))
        {
            return s_conf;
        }
        return "";
    }

    inline String GetConfigString()
    {
        return GetConfig();
    }

#ifdef QT_DLL
//     bool WriteToFile(const String& file_path/*, bool encode_base64 = false*/)
//     {
//         String s_conf;
//         if (rapidjson::SaveToString(jv_, s_conf, true, 2))
//         {
//             return qui::WriteTextFile(file_path, s_conf);
//         }
//         return false;
//     }

    inline KConfigValue& AddMember(const String& name, const QByteArray& val);
#endif

    /**
    *	�ڱ������õĹ����л�����value
    *      ��Ҫͨ���˷�������
    *
    **/
    inline KConfigValue* NewValue(rapidjson::Type typ = rapidjson::kObjectType)
    {
        KConfigValue *v = new KConfigValue(typ, this->cfg_);
        values_.push_back(v);
        return v;
    }

    inline KConfigValue& AddMember(const String& name, const char* val);

    inline KConfigValue& AddMember(const String& name, KConfigValue* val);
    
    inline KConfigValue& AddMember(const String& name, const String& val);

    inline KConfigValue& AddMember(const String& name, int val);

    inline KConfigValue& AddMember(const String& name, KConfig* cfg);

    template <class _typ>
    KConfigValue& AddMember(const String& name, _typ val);

    // inline KConfigValue& AddMemberBase64(const String& name, const String& val);

protected:
    bool Exist(KConfigValue* val)
    {
        for each(KConfigValue* v in values_)
        {
            if (v == val)
                return true;
        }
        return false;
    }

//     const StrType utf8_base64(const String& val)
//     {
//         String *v = new String();
//         *v = val.toBase64(String::Base64Encoding);
//         strs_.push_back(v);
// 
//         return StrType(v->constData());
//     }

protected:
    // real value
    rapidjson::Value*   jv_;

private:
    // �������е�value
    std::vector<KConfigValue*> values_;

    // �ĵ�
    KConfig* cfg_;
};

class KConfig : public KConfigValue
{
    typedef RDoc::AllocatorType Alloctor;
    typedef RDoc::Ch MyCh;

    KConfig(const KConfig &) = delete; 
    KConfig &operator=(const KConfig &) = delete;

public:
    KConfig()
    {
        jv_ = new rapidjson::Document;

        cfg_ = this;

        // Ĭ�Ͽ�
        doc()->Parse<0>("{}");
    }

    virtual ~KConfig()
    {
        delete doc();

        jv_ = nullptr;

//         for each(auto *p in sub_cfgs_)
//         {
//             delete p;
//         }
    }

protected:
    RDoc* doc()
    {
        return (RDoc*)(jv_);
    }

public:
    // �µ�ʵ������Ҫʹ��DeleteInstɾ��֮
//     static KConfig* NewInst()
//     {
//         return new KConfig;
//     }
// 
//     static void DeleteInst(KConfig* cfg)
//     {
//         delete cfg;
//     }

    // ��ǰ�����µ������ã������ֶ�ɾ��
//     QConfig* NewSubConfig()
//     {
//         QConfig* c = new QConfig;
//         sub_cfgs_.push_back(c);
//         return c;
//     }

    inline Alloctor& GetAlloctor()
    {
        return doc()->GetAllocator();
    }

#ifdef QT_DLL 
    bool ReadFromFile(const String& file_path)
    {
        QFile file(file_path);
        if (file.open(QFile::ReadOnly))
        {
            String all = file.readAll();
            doc()->Parse<0>(all.toUtf8().constData());
            if (doc()->HasParseError())
            {
                doc()->Parse<0>("{}");
                return false;
            }
            return true;
        }
        return false;
    }
#endif

};

template <class _typ>
KConfigValue& KConfigValue::AddMember(const String& name, _typ val)
{
    static_assert(!std::is_pointer<_typ>::value, "can't be pointer type!");

    // _typ ������value��config
    jv_->AddMember(_utf8_str(name), rapidjson::Value(val), cfg_->GetAlloctor());

    return *this;
}

KConfigValue& KConfigValue::AddMember(const String& name, int val)
{
    jv_->AddMember(_utf8_str(name), rapidjson::Value(val), cfg_->GetAlloctor());

    return *this;
}

KConfigValue& KConfigValue::AddMember(const String& name, const String& val)
{
    jv_->AddMember(_utf8_str(name), _utf8_str(val), cfg_->GetAlloctor());

    return *this;
}

KConfigValue& KConfigValue::AddMember(const String& name, KConfigValue* val)
{
    assert(nullptr != val);
    assert(Exist(val));

    jv_->AddMember(_utf8_str(name), *(val->jval()), cfg_->GetAlloctor());

    return *this;
}

KConfigValue& KConfigValue::AddMember(const String& name, KConfig* cfg)
{
    assert(nullptr != cfg);
    assert(cfg_->Exist(cfg));

    jv_->AddMember(_utf8_str(name), *(cfg->jval()), cfg_->GetAlloctor());

    return *this;
}

KConfigValue& KConfigValue::AddMember(const String& name, const char* val)
{
    assert(nullptr != val);
    
    jv_->AddMember(_utf8_str(name), _utf8_p(val), cfg_->GetAlloctor());

    return *this;
}

#ifdef QT_DLL
KConfigValue& KConfigValue::AddMember(const String& name, const QByteArray& val)
{
    jv_->AddMember(_utf8_str(name), _utf8_p(val.constData()), cfg_->GetAlloctor());

    return *this;
}
#endif

KConfigValue& KConfigValue::PushBack(KConfigValue* v)
{
    jv_->PushBack(*(v->jval()), cfg_->GetAlloctor());

    return *this;
}


#endif // config_h__
