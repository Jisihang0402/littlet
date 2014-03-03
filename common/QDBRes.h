#pragma once

#include "file/db/QDBMgr.h"
#include "ConstValues.h"
#include "time/QTime.h"
#include "AppHelper.h"

enum ENUM_DBFILE_TYPE
{
	DBFILE_TYPE_NONE = 0x0,
	DBFILE_TYPE_UNKOWN = 0x1,
	DBFILE_TYPE_IMAGE = 0x2,
	DBFILE_TYPE_TEXT = 0x4,
	DBFILE_TYPE_AUDIO = 0x8,
	DBFILE_TYPE_VIDEO = 0x10,
	DBFILE_TYPE_EXE = 0x20,
    DBFILE_TYPE_THUMBICON = 0x40,
	DBFILE_TYPE_ALL = 0xFF,
};

struct TFileData
{
	TFileData& operator=(const TFileData&o)
	{
		nID = o.nID;
		nDataSize = o.nDataSize;
//		bufData.Copy(o.bufData);
		nRefs = o.nRefs;
		sResName = o.sResName;
		sExt = o.sExt;
		tmCreate = o.tmCreate;
		tmUpdate = o.tmUpdate;
		eType = o.eType;

		return *this;
	}

	int			nID;
	QBuffer		bufData;
	DWORD		nDataSize;
	LONG		nRefs;		// ���ü���
	ENUM_DBFILE_TYPE	eType;
	QString		sResName;
	QString		sExt;		// �� '.'
	QTime		tmCreate;
	QTime		tmUpdate;
};

typedef std::vector<TFileData> VecFileData;
typedef VecFileData::iterator VecFileDataItr;

class QResMan;
class QResDB : public DBMan
{
    friend class QResMan;

	SINGLETON_ON_DESTRUCTOR(QResDB){}

private:
	BOOL Startup();

	// �� tbl_file ����
	BOOL AddData( __inout TFileData &fd );
	BOOL DeleteData(int nID);
    BOOL GetData( int nID, BOOL bWithData, __out TFileData& fd );
    BOOL AddDataRef(int nDataID);
	BOOL ReleaseDataRef(int nDataID);
	int GetDataRef(int nDataID);
	int GetDataCount();

	/** ����Data������ֶΣ�ID��size��name...)
	 *	-return:	
     *      ������Ŀ����
	 *	-params:	
	 *		-[in]	eType Ҫ��ȡ���ֶ���������
	 *		-[out]	vfd
	 **/
	int GetDataItems( __out VecFileData& vfd ,DWORD eType = DBFILE_TYPE_ALL);

	// bIncludeData ����BLOB����
	int GetDataItems(__in IntArray& vi,__out VecFileData& vfd,BOOL bIncludeData=FALSE);
};

// QResDB ������
class QResMan
{
    SINGLETON_ON_DESTRUCTOR(QResMan)
    {

    }

public:
    BOOL Startup(QString sIconDir);

    /** ����icon�����ݿ��е�ID��ȡ��·��
     *	return
     *      icon���ͷŵ���·��,
     *	param
     *		-[in]
     *          tfd 
    **/
    QString GetDBResFilePath(__in TFileData& tfd);
    QString GetDBResFilePath(__in int nID);

    /** �������ͷŵ�szFile
     *	return
     *      TRUE    �㶨
     *	param
     *		-[in]
     *          nID     ����ID
     *          szFile  �ļ�·��������Ϊȫ·��
    **/
    BOOL ReleaseDataToFile( __in int nID, __in LPCWSTR szFile );

    /**�Զ�������ݣ��������������ͷŵ��ض���Ŀ¼
     *	return
     *      �ͷŵ����ļ�·����ʧ�ܷ��ؿ�
     *	param
     *		-[in]
     *          nID     ����ID
    **/
    QString ReleaseDataToFile(__in int nID );

    int GetDataItems( __out VecFileData& vfd ,DWORD eType = DBFILE_TYPE_ALL);

    // bIncludeData ����BLOB����
    int GetDataItems(__in IntArray& vi,__out VecFileData& vfd,BOOL bIncludeData=FALSE);

    // �� tbl_file ����
    BOOL AddData( __inout TFileData &fd );
    BOOL DeleteData(int nID);
    BOOL GetData( int nID, BOOL bWithData, __out TFileData& fd );
    BOOL AddDataRef(int nDataID);
    BOOL ReleaseDataRef(int nDataID);
    int GetDataRef(int nDataID);
    int GetDataCount();

private:
    IntArray    m_vResIcons;    // �Ѿ��ͷŵ�cache�е�icon��id�ᱻ��¼���
    QString     m_sIconDir;
};


