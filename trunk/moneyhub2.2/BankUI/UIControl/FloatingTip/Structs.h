#pragma once
#ifndef _STRUCTORS_H
#define _STRUCTORS_H

#include <string>

enum eTimers
{
	eTimer_ToolTipDelayShow = 1 ,
	eTimer_BalloonHide ,
	eTimer_MouseLeave ,
	eTimer_FadeInOut ,
};

enum eCloseBtnState
{	
	eMouseOut = 0,
	eMouseOver ,
	eMouseDown
};

enum eCursor
{
	eCURSOR_DEFAULT = 0,
	eCURSOR_HAND
};

enum eToolTipFlag
{
	eTTF_CLOSE = 0x1 ,
	eTTF_BALLOON = 0x2 ,	// ����������
	eTTF_ICON = 0x4 ,
	eTTF_TITLE = 0x8 ,
	eTTF_TEXTCALLBACK = 0x10 , 
	eTTF_CUSTOMDRAW = 0x20 ,	// not supported
	eTTF_INDEXCALLBACK = 0x40 ,
	eTTF_CHECKBOX = 0x80,	// 
	eTTF_ADVANCE = 0x100,	// �߼�����
	eTTF_ROBADVANCE = 0x200,	// �����ǰ������ʾ����eTTF_ADVANCE,�����ǿ��ת��Ϊ!eTTF_ADVANCE������ʾ������
	eTTF_SHOWTIMEOUT = 0x400,
	eTTF_USERCLOSENOTIFY = 0x800, // ��������eTTF_BALLOON������û���X����֪ͨhWnd����
	eTTF_HIDDENNOTIFY = 0x1000,  // ������ʧ
	eTTF_FADEIN = 0x2000,	// �Ƿ���
	eTTF_FIXEDWIDTH = 0x4000,	// �̶����ݿ��
	eTTF_FIXEDHEIGHT = 0x8000,   // �̶����ݸ߶�
	eTTF_BALLOONPOPDOWN = 0x10000, // ����Ĭ�������ϵ��ģ����������־�������µ���ר��Ϊ�ӽ��̱������ġ� 
	eTTF_CHECKBOXTAKEEFFECTNOW = 0x20000	// ��ѡ����ĺ�������Ч��������֪ͨ������
};

enum eCheckBoxState
{
	eCBS_UnChecked = 0 ,
	eCBS_UnCheckedHover ,
	eCBS_UnCheckedActive ,
	eCBS_UnCheckedDisable,
	eCBS_Checked,
	eCBS_CheckedHover,
	eCBS_CheckedActive,
	eCBS_CheckedDisable,

};

enum eForward
{
	eLeft = 1 ,
	eTop , 
	eRight ,
	eBottom
};



typedef struct  tag_QuadPixel
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char alpha;
} QUADPIXEL , *PQUADPIXEL ;

//////////////////////////////////////////////////////////////////////////
// NMSTTDISPINFO

#define STTDSPF_ICON		0x1
#define STTDSPF_TITLE		0x2
#define STTDSPF_TEXT		0x4
#define STTDSPF_INDEX		0x8

typedef struct tag_NMSTTDISPINFO
{
	NMHDR hdr ;
	DWORD dwFlags;
	HICON hIcon ;
	SIZE IconSize ;
	DWORD_PTR pdwIndex ;
	std::wstring szTitle;
	std::wstring szText;
} NMSTTDISPINFO , *LPNMSTTDISPINFO;

typedef struct tag_NMSTTGETINDEX
{
	NMHDR hdr ; 
	DWORD_PTR pdwIndex ;
} NMSTTINDEX , *LPNMSTTINDEX;

typedef struct tag_NMSTTNEEDSHOWTIP
{
	NMHDR hdr ;
	BOOL bShowTip ;
} NMSTTNEEDSHOWTIP , *LPNMSTTNEEDSHOWTIP;

typedef struct tag_NMSTTLINKCLICK
{
	NMHDR hdr ;
	LPCTSTR lpUrl ;
} NMSTTLINKCLICK;


typedef struct  tag_SogouToolTipInfo
{
	UINT uFlags ; 
	HICON hIcon ;
	HBITMAP hBmp ;
	SIZE IconSize ;
	SIZE BalloonSize ;		// ���ݴ�С��ָ���ģ�����������Ӧ
	HWND hWnd ;	
	HWND hFollowWnd ;		// ���ݸ����ĸ�������
	UINT uCallBackMsg ;
	DWORD dwCheckBoxState;
	UINT uDelay ;	// �ӳ���ʾ
	UINT uShowTimeOut ; // �ȶ����ʧ
	DWORD_PTR pdwIndex ;
	std::wstring wstrTitle ;
	std::wstring wstrText ;
	std::wstring wstrCheckBox ;
} SOGOUTOOLINFO ,*LPSOGOUTOOLINFO ;


// uCallBackMsg ��Ϣ
//		wParam:	֪ͨ������ TTCBMT_*
//		lParam:	DWORD_PTR	��Ϣ��ֵ
#define TTCBMT_USERCLOSE		1	// �û�����X����������eTTF_USERCLOSENOTIFY
#define TTCBMT_CHECKBOXSTATE	2	// ��CheckBox����Ҫ�ص�֪ͨCheckBox��ѡ��״̬
#define TTCBMT_LINK				3	// ��������
#define TTCBMT_HIDDEN			4	// ������ʧ

typedef struct tag_ToolRect
{
	HWND hWnd ;
	UINT uFlags ;	// eTTF_TEXTCALLBACK , eTTF_ICON
	HICON hIcon ;
	SIZE IconSize ;
	DWORD_PTR pdwIndex ;
	RECT rc ;
	std::wstring wstrTitle ;
	std::wstring wstrText ;
} TOOLRECT , *LPTOOLRECT;



typedef struct tag_FadeInOut
{
	BOOL bOut ;
	BOOL bVaild ;
	int iPrecent ;
	int iStep ;
	SIZE bmpSize ;
	int iBufSize ;
	QUADPIXEL* pPixels ;
} FADEINOUT , *LPFADEINOUT;

#endif