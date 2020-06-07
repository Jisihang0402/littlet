#include "QResIconsDlg.h"
#include "../common/LittleTUIcmn.h"
#include "../common/ConstValues.h"
#include "../common/QDBRes.h"
#include "img/QImgProcess.h"
#include "file/FileOper.h"
#include "LittleT.h"

QUI_BEGIN_EVENT_MAP(QResIconsDlg, QDialog)
    BN_STATECHANGED_NAME(L"icon-item", &QResIconsDlg::OnIconSelChanged)
    BN_STATECHANGED_ID(L"path-newicon", &QResIconsDlg::OnAddNewIcon)
QUI_END_EVENT_MAP()

QResIconsDlg::QResIconsDlg(void)
    :QDialog(L"qabs:dlgs/icons.htm")
{
}

QResIconsDlg::~QResIconsDlg(void)
{
}

void QResIconsDlg::OnIconSelChanged( HELEMENT he )
{
    ECtrl eIcon(he);
    m_sIconPath = eIcon.get_attribute("src"); 
    m_nIconID = (int)eIcon.GetData();

    m_nRetID = IDOK;

    PostMessage(WM_CLOSE);
}

LRESULT QResIconsDlg::OnDocumentComplete()
{
    CStdString sIconDir = ((LittleTConfig*)QUIGetConfig())->GetIconsDir();

    QResMan* pMan = QResMan::GetInstance();
    // ��iconsĿ¼�µ������ļ���ɨ�����
    VecFileData vfd;
    QResMan::GetInstance()->GetDataItems(vfd, DBFILE_TYPE_THUMBICON);
    for (VecFileData::iterator i = vfd.begin(); i != vfd.end(); ++i)
    {
        AddIcon((*i).nID, pMan->GetDBResFilePath(*i));
    }

    OnAddNewIcon(NULL);

    return 0;
}

BOOL QResIconsDlg::AddIcon(int nIconID, LPCWSTR szIconPath )
{
    ECtrl eIcon = ECtrl::create("picture");
    _IconBox().append(eIcon);
    eIcon.set_attribute("src", szIconPath);
    eIcon.set_attribute("name", L"icon-item");
    eIcon.SetData((LPVOID)nIconID);

    return TRUE;
}

void QResIconsDlg::OnAddNewIcon(HELEMENT he)
{
    EFilePath ePath = GetCtrl("#path-newicon");
	CStdString sImg = ePath.GetFilePath().c_str();
//    QString sImg = L"c:\\Users\\Public\\Pictures\\Sample Pictures\\Tulips.jpg";
	sImg = sImg.Trim();
	if (sImg.IsEmpty())
	{
		return;
	}

    // ת��image���̶���С������ͼ
    QImgProcess qIP;
    if ( !qIP.OpenFile(sImg) )
    {
        XMsgBox::ErrorMsgBox(sImg + L"���ܴ�ͼ���ļ�");
        return;
    }
    Gdiplus::Image* pThumb = qIP.CreateThumb(ICON_THUMB_WIDTH, ICON_THUMB_HEIGHT, FALSE);
    if (NULL == pThumb)
    {
        XMsgBox::ErrorMsgBox(L"ת��ͼ��ʧ��");
        qIP.Close();
        return;
    }

    // �������С����ͼ
    CStdString sExt = quibase::CPath(sImg).GetExtension();
    CStdString sTmpFile = 
        ((LittleTConfig*)QUIGetConfig())->GetIconsDir() 
        + L"__temp__" + sExt;
    if ( !qIP.SaveIamge(pThumb, sTmpFile) )
    {
        XMsgBox::ErrorMsgBox(L"��������ͼʧ��!");
        delete pThumb;
        qIP.Close();
        return;
    }
    delete pThumb;
    qIP.Close();

    // ���������ͼд�뵽���ݿ�
    TFileData tfd;
    tfd.bufData.FileRead(sTmpFile);
    tfd.eType = DBFILE_TYPE_THUMBICON;
    tfd.nDataSize = tfd.bufData.GetBufferLen();
    tfd.nRefs = 0;
    tfd.sResName = L"icon";
    tfd.sExt = sExt.Mid(1);
    if ( !QResMan::GetInstance()->AddData( tfd ) )
    {
        XMsgBox::ErrorMsgBox(L"icon �������ʧ��");
        return;
    }

    // ��д����ͼ������
    CStdString sDest = QResMan::GetInstance()->GetDBResFilePath(tfd);
    ASSERT(!sDest.IsEmpty());
    MoveFile(sTmpFile, sDest);

    // ��ӵ�icon box
    AddIcon(tfd.nID, sDest);
}
