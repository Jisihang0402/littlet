#include "QNote.h"


QNote::QNote(void)
{
}


QNote::~QNote(void)
{
}

CStdString QNote::TypeString()
{
	CStdString sRet;
	switch (Type())
	{
	case NOTE_TYPE_NOTSET:
		{
			sRet = L"�ǹ����ʼ�";
			return sRet;
		}
	case NOTE_TYPE_GOALITEM:
		{
			sRet = L"�ƻ�Ŀ����";
			break;
		}
	}
	CStdString sID;
	sID.Format(L"[%d]",ParentID());
	sRet += sID;
	return sRet;
}
