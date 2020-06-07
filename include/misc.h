#ifndef misc_h__
#define misc_h__

#include "stdstring.h"

namespace littlet
{
    /**
     *	�첽����������	
     *
     *  name    �������� login / logout / feed
     *  bAsync  TRUE    �첽����
    **/
    bool SendWebRequest(const CStdString& name, const CStdString& content, BOOL bAsync = TRUE);

    // mac ��ַ
    bool GetMACAddress(CStdString& mac_addr);
}


#endif // misc_h__
