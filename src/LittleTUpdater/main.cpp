#include <iostream>
#include <vector>

// ����console����
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"wmainCRTStartup\"") 

#include "../common/LittleTcmn.h"

#include "QBuffer.h"
#include "crypt/CXUnzip.h"
#include "deps/sys/UACSElfElevation.h"

#include <atlbase.h>
#include <atlstr.h>

using namespace std;

// #ifndef _DEBUG
// #define  _DEBUG   1
// #endif
#ifdef _DEBUG
#pragma comment(lib, "quil_mtd")
#else
#pragma comment(lib, "quil_mt")
#endif

BOOL IsFileExist(LPCTSTR pszFile)
{
    BOOL bRet = FALSE;
    if( pszFile == NULL )
        return bRet;
    if( pszFile[0] == 0 )
        return bRet;

    WIN32_FIND_DATA fd = {0};
    HANDLE hFile = FindFirstFile(pszFile, &fd);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
        if( !(fd.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) )
            bRet = TRUE;
    }
    return bRet;
}

struct _Files 
{
    wstring     sdest;
    wstring     sbackup; // �����ļ�
    wstring     ssrc;   // Դ�ļ�
    QBuffer     buf;
};

typedef std::vector<_Files*> LstFiles;

/** ��ѹѹ�����ڵ��������ݲ����浽����
 *	return:
 *      true    �ɹ�
 *	params:
 *		-[in]
 *          szZip   zip�ļ�
 *          lst     �ļ���Ϣ
**/
bool ExtractAll(__in LPCWSTR szZip, __out LstFiles& lst)
{
    lst.clear();
    // ��ѹѹ����
    CXUnzip theZip;
    if ( !theZip.Open(szZip) )
    {
        return false;
    }

    bool bRet = true;
    // ����zip��������ļ�
    ZIPENTRYW ze;
    for (int i = 0; i < theZip.GetItemCount(); ++i)
    {
        if (!theZip.GetItemInfo(i,&ze))
            continue;

//         if (StrCmpIW(LITTLET_UPDATE_CONTENT_FILENEWNAME, ze.name) == 0)
//             continue;   // �������ݣ�����д

        _Files* p = new _Files;
        p->sdest = ze.name;
        if ( !theZip.UnzipToBuffer(i, p->buf) )
        {
            delete p;
            bRet = false;
            break;
        }
        lst.push_back(p);
    }
    if (!bRet)
    {
        for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
        {
            delete *i;
        }
        lst.clear();
    }
    theZip.Close();
    return bRet;
}

bool UpdateFiles(__in LstFiles& lst, __in wstring sDest)
{
    bool bRet = true;
    for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
    {
        _Files* p = *i;
        p->ssrc = sDest;
        p->ssrc += p->sdest;
        p->sbackup = p->ssrc;
        p->sbackup += L".__bak__";
        // �ȱ�������ļ�
        if (!IsFileExist(p->ssrc.c_str()))
        {
            // ԭ�������ڵ��ļ�����ֱ�ӿ������ļ��������
            p->sbackup.clear();
        }
        else
        {
            // ���е��ļ���Ҫ�ȱ�����
            // ����Ǹ����Լ���LittleTUpdater.exe��, �Ǿ�ֱ�����������LittleT�������Լ�
            // ֻ��Ҫ���Լ�д����ǰ�ļ�������ľ�����
            if (StrCmpIW(p->sdest.c_str(), L"LittleTUpdater.exe") == 0)
            {
                // �����̡�LittleT.exe��������ʱ�������û������ļ�
                // �еĻ����ͻ������滻����ǰ��LittleTUpdater.exe
                p->buf.FileWrite(LITTLET_UPDATER_NEWNAME);
                continue;
            }
            // ������Ǹ��¡����³��򡱣���ô�ȱ���
            if (!MoveFileEx(p->ssrc.c_str(), p->sbackup.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
#ifdef _DEBUG
                wcout<<"backup File��["
                    <<p->ssrc.c_str()<<"] to ["
                    <<p->sbackup.c_str()<<"] failed!"<<endl;
#endif
                p->ssrc.clear();
                p->sbackup.clear();
                bRet = false;
                break;
            }
        }
        // ���ļ�����д������
        if ( !p->buf.FileWrite(p->ssrc.c_str()) )
        {
#ifdef _DEBUG
            wcout<<"Write File��["<<p->ssrc.c_str()<<"] failed!"<<endl;
#endif
            bRet = false;
            break;
        }
    }
    return bRet;
}

// ����û��ɣ��ӱ����лָ�ԭ�����ļ�
bool RestoreFile(__in LstFiles& lst)
{
    bool bRet = true;
    for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
    {
        _Files* p = *i;
        if ( !p->sbackup.empty() )
        {
            // ��3�λָ�
            int j = 0;
            while (j < 3)
            {
                if (MoveFileEx(p->sbackup.c_str(), p->ssrc.c_str(),
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                {
                    break;
                }
                j++;
            }
            if (j == 3)
            {
                // fuck!
                // ����Ӧ�ñ��ƻ���
                bRet = false;
            }
            else
            {
                p->sbackup.clear();
                p->ssrc.clear();
            }
        }
    }
    return bRet;
}

// �ļ����³ɹ��������ļ�����ɾ����
void DeleteBackup(__in LstFiles& lst)
{
    for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
    {
        if (!(*i)->sbackup.empty())
        {
            ::DeleteFile((*i)->sbackup.c_str());
        }
    }
}

// argv[1]  zip path
// argv[2]  dest path
int wmain(int nArgc, wchar_t**argv)
{
#ifdef _DEBUG
    for (int i = 0; i < nArgc; i++)
    {
        wcout<<L"arg "<<i<<":  "<<argv[i]<<endl;
    }
#endif

    if (nArgc < 3)
    {
        wcout<<"�������󣡸���ʧ��"<<endl;
        return -1;
    }

    wcout<<"���ڸ��£����Ե�Ƭ�̣���Ҫ�ر�������ڣ�Ӧ�ò��ᳬ��10���....."<<endl;
    // ȷ���������Ѿ��˳���
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, LITTLET_UPDATER_EVENTNAME);
    if (NULL != hEvent)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            // �ȴ�����¼���Ϊ���ź���ִ��
            WaitForSingleObject(hEvent,3000);
        }
        CloseHandle(hEvent);
    }
    // �ͷŵ�Ŀ��·��
    wstring sZip = argv[1];
    wstring sDestDir = argv[2];
    bool bFailed = false;
    do 
    {
        // ��ѹ�����ļ�
        LstFiles lst;
        //    wstring szip = L"f:\\UI\\ddd\\UI.zip";
        wcout<<"��ѹ�ļ�....."<<endl;
        if ( !ExtractAll(sZip.c_str(), lst) )
        {
            wcout<<"��ѹ�ļ����󣬸���ʧ��"<<endl;
            bFailed = true;
            break;
        }

        // �����Լ�������Ȩ�ޣ���Ӧ��vista������ϵͳ��UAC����
        CUACSElfElevations::SelfElevation();

        // ��ʼ�滻�ļ�
        wcout<<"���³���....."<<endl;
        if ( !UpdateFiles(lst, sDestDir))
        {
            wcout<<"�Σ�������˼�������ļ�ʧ�ܣ�"<<endl;
            if (!RestoreFile(lst))
            {
                wcout<<"-_-! ��������˼�ˣ����ָܻ���ԭ��״̬��"<<endl;
                wcout<<"���ֶ�ȥ��__bak__��չ�����������-->���ʶ����Ե���ô��"<<endl;
                // fuuuuuuuuuuuuuuuuuuck!!!!!!
            }
            bFailed = true;
        }
        else
        {
            // ok�����³ɹ�
            // ɾ�������ļ�
            DeleteBackup(lst);
        }

        // ������Դ
        for (LstFiles::iterator i = lst.begin(); i != lst.end(); ++i)
        {
            delete *i;
        }
        lst.clear();
    } while (false);

    // ���µ��м��ļ�Ҳɾ����
    DeleteFile(sZip.c_str());

    if (bFailed)
    {
        system("pause");
    }
    else
    {
        wcout<<"�㶨��..."<<endl;
    }

    // ������Σ�������������������
    wstring sfile = sDestDir;
    sfile += L"LittleT.exe";
    ShellExecute(NULL,L"open", sfile.c_str(), NULL, sDestDir.c_str(), SW_SHOWNORMAL);

    return 0;
}

