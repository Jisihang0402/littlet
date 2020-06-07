#include "file/FileEnumerator.h"
#include <list>
#include <iterator>
#include "StkTimerMan.h"
#include "../common/ConstValues.h"
#include "AppHelper.h"
#include "ui/QUIMgr.h"

class QPictureLoader
{
public:
	QPictureLoader()
	{
		m_hTimer = NULL;
		m_nIdx = -1;
		m_bRecycle = FALSE;
	}
	/*
	 *	ɨ���ļ����µ�ͼ���ļ���*.jpg,*.png��
	 ��	-return
			ɨ����ļ���Ŀ
	 */
	int LoadFolder(__in LPCWSTR szFolder)
	{
		StopAutoPlayTimer();

		m_nIdx = 0;
		m_bRecycle = FALSE;
		m_hHtmlayoutWnd = NULL;
		m_sPath = szFolder;
		if (m_sPath.Find(L':') == -1)   // ��ȫ·��
        {
            m_sPath = quibase::GetModulePath() + m_sPath;
        }

		m_list.clear();

		QFileFinder ff(m_sPath,L"*.jpg;*.png;");
		FileList lst;
		ff.GetFileList(lst);
		std::copy(lst.begin(),lst.end(),
			std::back_insert_iterator<std::vector<tstring> >(m_list));

		return GetFileCount();
	}

	int GetFileCount() const
	{
		return m_list.size();
	}

	/*
	 *	����ɨ�赽���ļ�
	 * -param
		-[in]	szDestUri Htmlayout�ܹ�ʶ��ĵ�ַ������·����Ҫ�� "file://" ��ʼ
		-[in]	nPeriodSec	�Զ����ŵļ��ʱ�䣬��λΪ�롾s��
		-[in]	bAutoPlay	�Ƿ��Զ�����
		-[in]	bRecycle	�Ƿ�ѭ������
	 */
	BOOL HtmlayoutPlay(HWND hHtmlayoutWnd,LPCWSTR szDestUri,int nPeriodSec,BOOL bAutoPlay,BOOL bRecycle)
	{
		if (StrCmpNICW(szDestUri,L"file://",7) != 0)
		{ // ����ʶ��ĵ�ַ
			return FALSE;
		}
		if (QUIMgr::QWindowFromHWND(hHtmlayoutWnd) == NULL )
		{ // �ǿ�ʶ��Ĵ���
			return FALSE;
		}
		m_hHtmlayoutWnd = hHtmlayoutWnd;
		m_sDestUri = szDestUri;
		// ���磺 file://F:/phto/x.jpg . ����ʹ�� ��\\���ָ���
		m_sDestUri.Replace(L'\\',L'/');
		//m_sDestUri.Replace(L"//",L"/");

		m_bRecycle = bRecycle;
		// �����Զ����ŵĶ�ʱ��
		if (bAutoPlay && (GetFileCount() > 0))
		{
			return SetAutoPlayTimer(nPeriodSec);
		}
		return FALSE;
	}

	static VOID CALLBACK PlayCallback(
		__in  PVOID lpParameter,
		__in  BOOLEAN TimerOrWaitFired )
	{
		QPictureLoader *pThis = reinterpret_cast<QPictureLoader*>(lpParameter);
		if (NULL != pThis)
		{
			pThis->AutoPlayTimerActived();
		}
	}

	BOOL PlayPrevious()
	{
		int nFile = GetFileCount();
		if (nFile <= 0)
			return FALSE;

		if (m_nIdx <= 0)
			return FALSE;	// �Ѿ����ʼ��

		--m_nIdx;

		return PlayPicture(m_list[m_nIdx].c_str());
	}

	BOOL PlayNext()
	{
		int nRet = StepNext();
		if (0 == nRet)
			return PlayPicture(m_list[m_nIdx].c_str());
		else if(1 == nRet)
			; // ֪ͨ�������
		else 
			; // ����
	}

	BOOL PlayPicture(LPCWSTR szPic)
	{
		QBuffer buf;
		if (buf.FileRead(szPic))
		{
			BOOL bOK =  HTMLayoutDataReadyAsync(m_hHtmlayoutWnd,m_sDestUri,
				buf.GetBuffer(),buf.GetBufferLen(),HLRT_DATA_IMAGE);
			return bOK;
		}
		return FALSE;
	}
	
	BOOL PlayPicture(int idx)
	{
		if ((idx < 0) || (idx >= GetFileCount()))
		{
			return FALSE;
		}
		m_nIdx = idx;
		return PlayPicture(m_list[m_nIdx].c_str());
	}

    void StopPlay()
    {
        StopAutoPlayTimer();
    }
protected:
	
    BOOL SetAutoPlayTimer(int nSec)
	{
		StopAutoPlayTimer();

        m_hTimer = StkSetTimer(nSec * 1000, QPictureLoader::PlayCallback,this);

		return (NULL != m_hTimer);
	}
	
    void StopAutoPlayTimer()
	{
		if (NULL != m_hTimer)
		{
            StkKillTimer(m_hTimer,FALSE);
            m_hTimer = NULL;
		}
	}
	
    void AutoPlayTimerActived()
	{
		StepNext();

		QUIPostCodeTo(m_hHtmlayoutWnd,PICTRUELOADER_PLAYTIMER_FIRED,m_nIdx);
	}

	// -1 ֹͣ
	// 0 ����
	// 1 ��������
	int StepNext()
	{
		int nFile = GetFileCount();
		if (nFile <= 0)
        {
            StopAutoPlayTimer();
            return -1;
        }

		if (m_nIdx >= (nFile-1))
		{
			if (m_bRecycle)
			{
				m_nIdx = 0;
				return 0;
			}
			else
			{
				StopAutoPlayTimer();
				return 1;
			}
		}
		else
		{
			++m_nIdx;
		}
		return 0;
	}

public:
	CStdString		m_sDestUri;		// Htmlayout�ܹ�ʶ��ĵ�ַ������·����Ҫ�� "file://" ��ʼ
	HWND		m_hHtmlayoutWnd;
	BOOL		m_bRecycle;	// ѭ������
	CStdString		m_sPath;
	std::vector<tstring> m_list;
	int			m_nIdx;	// ��ǰ���ŵ��ļ�����
	HANDLE		m_hTimer;	// �Զ����ŵĶ�ʱ��
};

