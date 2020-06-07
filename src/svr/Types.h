#pragma once

#include "BaseType.h"

#include <vector>
#include <list>
#include <map>
#include "time/QTime.h"

// ����Լ����
// struct T - data struct
// class D - data class
// class C - UI class
// class I - interface class

typedef DWORD IDExtResType;

#ifndef INVALID_ID
#define INVALID_ID (-1)
#endif

enum
{
    INVALID_ID_VAL = -1,
};

// ���е����ݿ���Ӧ�����ݽṹ������̳��Դ˽ṹ
struct IDataItem
{
public:
    IDataItem()
    {
        m_nID = INVALID_ID;
    }

    // ������ID
    int      m_nID;
    // �������ʱ��
    QTime       m_tmCreation;

    // ɾ��
    virtual void OnDelete() 
    {
   
    }

    // �Ƴ�
    virtual void OnRemove()
    {

    }

    int& ID()
    {
        return m_nID;
    }

    QTime& CreationTime()
    {
        return m_tmCreation;
    }

    // bWithTime    �Ƿ���time
    CStdString CreationTimeDes(BOOL bWithTime = FALSE)
    {
        if (bWithTime)
            return CreationTime().Format(L"%Y/%m/%d %H:%M:%S");
        else
            return CreationTime().Format(L"%Y/%m/%d");
    }

    // �Զ����ʽ
    CStdString CreationTimeDes(LPCWSTR szFormat)
    {
        return CreationTime().Format(szFormat);
    }
};
