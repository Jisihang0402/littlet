#pragma once

#include <GdiPlus.h>

class QBuffer;
class QImgProcess
{
public:
	QImgProcess(void);
	~QImgProcess(void);

	// �µĿ�ߣ��п��ܱ���
	// bFailIfSmall = TRUE,�����ǰͼƬ�Ŀ�߶�С��Ҫ����������ͼ��С��
	//			��ô�Ͳ���������ͼ
	// �����´���������ͼָ�룬���ָ��Ҫ�ֶ�ɾ����ʹ��delete
	Gdiplus::Image* CreateThumb( int cx ,int cy,BOOL bFailIfSmall);
	// ���ֿ�߱�
	Gdiplus::Image* CreateThumb( int cx ,BOOL bFailIfSmall);
	// open image file
	BOOL OpenFile(LPCTSTR sFile);
	void Close();

	BOOL Save(LPCTSTR pszPath,int quality =100);
// 	BOOL TransformFormat(LPCTSTR pszFormat,int quality=100);
// 	BOOL GetImageData( __out QBuffer& buf );
// 	DWORD GetImagePhysicSize();

public:
	static BOOL SaveIamge(Gdiplus::Image *pImg,LPCTSTR pszPath,int quality =100);
// 	static BOOL GetImageData( Gdiplus::Image*pImg,__out QBuffer& buf );
// 	static BOOL TransformFormat(Gdiplus::Image *pImg,LPCTSTR pszFormat,int quality=100);
// 	static DWORD GetImagePhysicSize(Gdiplus::Image* pImg);
private:
	Gdiplus::Image*		m_pImg;
};
