#ifndef misc_h__
#define misc_h__

#include "stdstring.h"

namespace littlet
{
    // ��������
    bool SendWebRequest(const CStdString& name, const CStdString& content);

    // mac ��ַ
    bool GetMACAddress(CStdString& mac_addr);
}


#endif // misc_h__
