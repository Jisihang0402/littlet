#pragma once

#include "ui/QWindow.h"

class QResIconsDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP

public:
    QResIconsDlg(void);
    ~QResIconsDlg(void);

protected:
    virtual LRESULT OnDocumentComplete();

    void OnIconSelChanged(HELEMENT);
    void OnAddNewIcon(HELEMENT he);

    // nIconID  Ϊͼ�������ݿ��е�ID
    // szIconPath Ϊ�Ѿ�ת���õ�Сͼ���·��
    BOOL AddIcon(int nIconID, LPCWSTR szIconPath );

private:
    ECtrl _IconBox()
    {
        return GetCtrl("#icon-xbox");
    }

public:
    QString     m_sIconPath;
    int         m_nIconID;
};

