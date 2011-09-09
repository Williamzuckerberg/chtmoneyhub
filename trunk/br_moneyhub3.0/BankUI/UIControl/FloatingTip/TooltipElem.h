#pragma once

#ifndef _TOOLTIPTEXT_H
#define _TOOLTIPTEXT_H
//////////////////////////////////////////////////////////////////////////
// ������ʾ�ĸ�ʽ���£�
// ��<item type='text' color='0x00FF00' >����</item>�������ӣ�<item type='wrap' ></item>
// <item type='link' id='1' bold='0' url='http://www.sohu.com' color='0x000000' hover='0x0000FF' active='0xFF0000'>�Ѻ���ҳ</item> 
// ת�壺
//		&lt;    <
//		&gt;    >
//		&amp;   &
#include <map>
#include <string>
using namespace std;

#define TIT_BEGIN	0x0

#define TIT_UNKNOWN	0x0
// �ı�����
#define TIT_TEXT	0x1
// ��������
#define TIT_LINK	0x2
// ���з�
#define TIT_RETURN	0x3

#define TIT_END		0x03 

typedef union unItemData
{
	bool bWrap ;		// �Ƿ���
	struct ItemText
	{
		COLORREF clr;
		wstring wsText ;
	}* pitText;	// ����
	struct ItemLink
	{
		COLORREF clrNormal;
		COLORREF clrHover;
		COLORREF clrActive ;
		BOOL bBold ;
		int iId ;
		wstring wsText ;
	} * pilLink;	// ����
	void* pVoid ;
} TEXTITEMDATA;

typedef struct tagLinkItemBlock
{
	COLORREF clrNormal;
	COLORREF clrHover;
	COLORREF clrActive ;
	BOOL bBold ;
	RECT rcBlock ;
	int iId ;
	wstring wsText ;
} LINKBLOCK;

class CItemData
{
	struct _Data;
public:
	CItemData() ;
	CItemData(DWORD dwType) ;
	CItemData(const CItemData& cti) ;
	~CItemData();

	CItemData& operator = ( const CItemData& cit ) ;

public:
	RECT GetRect(CDCHandle& dc) ;
	DWORD GetType();
	wstring GetText() ;
	int GetId() ;
	COLORREF GetColor() ;
	COLORREF GetColorHover() ;
	COLORREF GetColorActive() ;
	BOOL GetBold() ;
	bool SetType(DWORD dwType) ;
	bool SetText(const wstring& wsText) ;
	bool SetId(int id) ;
	bool SetColor(COLORREF clr) ;
	bool SetColorHover(COLORREF clr) ;
	bool SetColorActive(COLORREF clr) ;
	bool SetBold(BOOL bBold) ;

	typedef CItemData::_Data _Data;

private:
	bool x_AllocBuffer(DWORD dwType) ;
	_Data* x_Alloc(DWORD dwType) ;
	bool x_ReleaseBuffer() ;
	bool x_CopyOnWrite() ;
	
private:
	struct _Data
	{
		int iRef;
		DWORD dwType ;// �������:TIT_TEXT,TIT_LINK , TIT_RETURN
		TEXTITEMDATA* pData ;
	}* m_pData;
};


// �ڵ��࣬����parseÿһ���ڵ�
class CItem
{
public:
	CItem() ;
	CItem(LPCTSTR& pItem) ;
	bool SetItem(LPCTSTR& pItem) ;
	bool IsGood() ;
	bool IsItemEnd() ;
	DWORD GetType() ;
	wstring GetAttribString(const wstring& wsAttr) ;
	COLORREF GetAttribColor(const wstring& wsAttr) ;
	int GetAttribInt(const wstring& wsAttr) ;
private:
	bool x_Parse(LPCTSTR& pItem) ;
	bool x_GetAttribValue(LPCTSTR& pItem , wstring& wsValue) ;
private:
	bool m_bGood ;
	bool m_bItemEnd ;
	typedef map<wstring , wstring> Attribute;
	Attribute m_Attributes ;
};

class CTooltipText
{
public:
	CTooltipText();
	bool SetText(const wstring& wsText) ;
	RECT GetRect(CDCHandle& dc , int iRowSapce = 4) ;
	bool RanderText(CDCHandle& dc, RECT rcRect , int iRowSapce = 4) ;
	list<CItemData>& GetItemList() ;
	list<LINKBLOCK>& GetLinkList() ;
private:
	int x_GetLineHeight(CDCHandle& dc) ;
	bool x_Parse(LPCTSTR& pText) ;
	size_t x_FindItem(const wstring& wsText) ;
private:
	list<CItemData> m_ItemList ;
	list<LINKBLOCK> m_LinkBlockList ;
};

// �㴰���ϵĿؼ�
class CLayeredCtrlBase
{
public:
	wstring GetWindowText() ;
	CLayeredCtrlBase* GetParent() ;
	bool SetWindowText(LPCTSTR lpText) ;
	
private:
	wstring m_wsWindowText ;
	RECT m_WndRect ;
	CLayeredCtrlBase* m_pParent ;
	HWND m_hMsgWnd ;
};

class CTooltipCheckbox : public CLayeredCtrlBase
{
public:
};

class CLayeredButton:public CWindowImpl<CLayeredButton , CButton>
{
	
};

#endif