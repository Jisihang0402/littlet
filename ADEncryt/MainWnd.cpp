#include "MainWnd.h"
#include "../common/LittleTUIcmn.h"
#include "ui/QUIMgr.h"
#include "ui/QUIGlobal.h"
#include "ui/quimgr.h"
#include "crypt/QDataEnDe.h"
#include "file/FileOper.h"
#include "crypt/CXUnzip.h"
#include "file/FileEnumerator.h"

QUI_BEGIN_EVENT_MAP(MainWnd,QFrame)
	BN_CLICKED_ID(L"btn_encrypt",&MainWnd::OnBtnEncrypt)
	BN_CLICKED_ID(L"btn_decrypt",&MainWnd::OnBtnDecrypt)
QUI_END_EVENT_MAP()

MainWnd::MainWnd(void)
	:QFrame(L"qabs:Main.htm")
{
}

MainWnd::~MainWnd(void)
{
}

BOOL MainWnd::Startup()
{
	return QFrame::Create(NULL);
}

BOOL MainWnd::OnClose()
{
    SetMsgHandled(FALSE);
	if (XMsgBox::YesNoMsgBox(L"ȷ��Ҫ�˳�����ô��") == IDYES)
	{
		::DestroyWindow(GetSafeHwnd());
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

BOOL MainWnd::on_key( HELEMENT he, HELEMENT target, 
	UINT event_type, UINT code, UINT keyboardStates )
{
	if (KEY_UP == event_type)
	{
		if (VK_RETURN == code)
		{
			ECtrl ctl(he);
			if (aux::wcseqi(ctl.get_attribute("id"),L"search-bar"))
			{ 
				return TRUE;
 			}
		}
	}
	return FALSE;
}

BOOL MainWnd::ZipData(__in const QString&sDataPath,__out QBuffer &bufData)
{
	// ɨ���ļ�
	QBuffer bufFile;
	QFileFinder ff(sDataPath,L"*.*");
	FileList fileList;
	ff.GetFileList(fileList);

	// ����ѹ���ļ�
	CXZip zip;
	QString sTempZip = sDataPath + L"/__temp_file.zip";
	if (!zip.Create(sTempZip))
		return FALSE;
	// д��ѹ���ļ�
	for (FileListItr itr = fileList.begin();
		itr != fileList.end(); ++itr)
	{
		if (!bufFile.FileRead((*itr).c_str()))
		{
			return FALSE;
		}
		zip.AddFile(CPath((*itr).c_str()).GetFileName(),
			bufFile.GetBuffer(0),bufFile.GetBufferLen());
		bufFile.ClearBuffer();
	}
	zip.Close();
	
	BOOL bRet = bufData.FileRead(sTempZip);
	DeleteFile(sTempZip);
	return bRet;
}

void MainWnd::OnBtnEncrypt( HELEMENT )
{
	QString sPicPath = EFilePath(GetCtrl("#path_picture")).GetFilePath();
	QString sDataPath = EFolderPath(GetCtrl("#path_data")).GetFolderPath();
	
	ECtrl eInfo = GetCtrl("#infobox");
	if (sPicPath.IsEmpty() || sDataPath.IsEmpty())
	{
		eInfo.SetText(L"����д��������Ϣ");
		return;
	}

	QBuffer bufPic;
	if (!bufPic.FileRead(sPicPath))
	{
		eInfo.SetText(L"ͼ���ļ���ȡ����");
		return ;
	}
	DWORD dwPicSize = bufPic.GetBufferLen();
	if (dwPicSize < 2048)
	{
		eInfo.SetText(L"ͼ���ļ�̫С��");
		return;
	}

	ENumber ePicOffset = GetCtrl("#input_pic_offset");
	ePicOffset.SetLimit(0,bufPic.GetBufferLen() / 2);
	ENumber eKeyLen = GetCtrl("#input_pic_keylen");

	// ��ȡ�ļ����µ������ļ�
	// ������ѹ���ļ�	
	QBuffer bufData;
	if (!ZipData(sDataPath,bufData))
	{
		eInfo.SetText(L"��������ѹ���ļ�����");
		return;
	}
	DWORD dwOriginLen = bufData.GetBufferLen();
	DWORD dwOffset = ePicOffset.GetNum();
	DWORD dwKeyLen = eKeyLen.GetNum();
	QDataEnDe ende;
	if (!ende.SetCodeData(bufPic.GetBuffer(dwOffset),dwKeyLen))
	{
		eInfo.SetText(L"������������ʧ��");
		return;
	}
	if (!ende.EncryptData(bufData))
	{
		eInfo.SetText(L"��������ʧ��!");
		return;
	}
	DWORD dwEncrypDataLen = bufData.GetBufferLen();

	if (!bufPic.Write(bufData.GetBuffer(),dwEncrypDataLen))
	{
		eInfo.SetText(L"д������ʧ��");
		return;
	}
	// ���д��20���ֽڣ�˳������
	// ԭʼ���ݳ���
	bufPic.Write((BYTE*)&dwOriginLen,sizeof(DWORD));
	// ���ܺ�����ݳ���
	bufPic.Write((BYTE*)&dwEncrypDataLen,sizeof(DWORD));
	// ͼ���С
	bufPic.Write((BYTE*)&dwPicSize,sizeof(DWORD));
	// ��Կ����
	bufPic.Write((BYTE*)&dwKeyLen,sizeof(DWORD));
	// ��Կ��ͼ�������е�ƫ��
	bufPic.Write((BYTE*)&dwOffset,sizeof(DWORD));

	if (!bufPic.FileWrite(sDataPath + L"/" + CPath(sPicPath).GetFileName()))
	{
		eInfo.SetText(L"�����ļ�ʧ��!");
		return;
	}
	eInfo.SetText(L"ȫ���㶨��������ʹ��");
}

void MainWnd::OnBtnDecrypt( HELEMENT )
{
	QString sPicPath = EFilePath(GetCtrl("#path_picture")).GetFilePath();
	QString sSavePath = EFolderPath(GetCtrl("#path_data")).GetFolderPath();

	ECtrl eInfo = GetCtrl("#infobox");
	if (sPicPath.IsEmpty() || sSavePath.IsEmpty())
	{
		eInfo.SetText(L"����д��������Ϣ");
		return;
	}

	QBuffer bufPic;
	if (!bufPic.FileRead(sPicPath))
	{
		eInfo.SetText(L"ͼ���ļ���ȡ����");
		return ;
	}
	DWORD dwPicSize = bufPic.GetBufferLen();
	if (dwPicSize < 2048)
	{
		eInfo.SetText(L"����ļ���ͼ���ļ�");
		return;
	}


	// ���д��20���ֽڣ�˳������

	// ��Կ��ͼ�������е�ƫ��
	DWORD dwOffset;
	bufPic.ReadLast((BYTE*)&dwOffset,sizeof(DWORD));
	// ��Կ����
	DWORD dwKeyLen;
	bufPic.ReadLast((BYTE*)&dwKeyLen,sizeof(DWORD));
	// ͼ���С
	bufPic.ReadLast((BYTE*)&dwPicSize,sizeof(DWORD));
	// ���ܺ�����ݳ���
	DWORD dwEncrypDataLen;
	bufPic.ReadLast((BYTE*)&dwEncrypDataLen,sizeof(DWORD));
	// ԭʼ���ݳ���
	DWORD dwOriginLen;
	bufPic.ReadLast((BYTE*)&dwOriginLen,sizeof(DWORD));

	QBuffer bufData;
	if (!bufData.AllocBuffer(dwEncrypDataLen))
	{
		eInfo.SetText(L"�����ڴ����");
		return;
	}
	if (!bufData.Write(bufPic.GetBuffer(dwPicSize),dwEncrypDataLen))
	{
		eInfo.SetText(L"��ȡ���ݴ���");
	}
	
	QDataEnDe ende;
	if (!ende.SetCodeData(bufPic.GetBuffer(dwOffset),dwKeyLen))
	{
		eInfo.SetText(L"������������ʧ��");
		return;
	}
	if (!ende.DecryptData(bufData))
	{
		eInfo.SetText(L"��������ʧ��!");
		return;
	}
	QString sFileName = L"/__Data__.dat";
	if (!bufData.FileWrite(sSavePath + sFileName))
	{
		eInfo.SetText(L"��������ʧ��");
		return;
	}

	eInfo.SetText(L"ȫ���㶨�����Ѿ�����Ϊ��" + sFileName);
}

