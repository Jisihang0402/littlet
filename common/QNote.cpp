#include "QNote.h"


QNote::QNote(void)
{
}


QNote::~QNote(void)
{
}

QString QNote::TypeString()
{
	QString sRet;
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
	QString sID;
	sID.Format(L"[%d]",ParentID());
	sRet += sID;
	return sRet;
}
