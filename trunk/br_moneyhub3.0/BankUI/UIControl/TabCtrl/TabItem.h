#pragma once
#include <tuodwmapi.h>
#include "../../../Utils/ListManager/ListManager.h"
#include "../../Skin/SkinManager.h"
#include "TabCtrl.h"


class CTuotuoTabCtrl;
class CCategoryItem;
class CChildFrame;

typedef std::vector<std::tstring> StrArray;


enum PageType
{
	PageType_Normal,
	PageType_Blank,
	PageType_SpecialPage
};

enum SpecialPageType
{
	SpecialPage_Null,
	SpecialPage_StartPageSelector,
	SpecialPage_StartPage,
	SpecialPage_TabStartPage,
	SpecialPage_StartPageNavi,
	SpecialPage_TabStartPageNavi,
	SpecialPage_StartPageSearch,
	SpecialPage_TabStartPageSearch,
	SpecialPage_StartPageFavor,
	SpecialPage_TabStartPageFavor,
	SpecialPage_TabStartPageBlank,
};

enum StartPageType
{
	StartPage_Null = -1,
	StartPage_Navigate = 0,
	StartPage_Favorite,
	StartPage_Search
};


class CTabItem
{

public:

	CTabItem(bool bNoClose);

	// export function

	void ChangeItemParent(CTuotuoTabCtrl *pTabCtr);

	// ��tab��iDeltaHeight��ʾ��chromeƤ���£���ǩ�����ϲ��ճ����ĸ߶�
	void DoPaint(CDCHandle dc, ButtonStatus eTabStatus, Position btnPos, ButtonStatus eBtn, bool bDrawCloseBtn, int iDeltaHeight, bool bAero);
	bool NeedRefreshLoadingIcon() const;

	void SetToDefault();
	void SetTabView(CChildFrame *pFrame)			{ m_pChildFrame = pFrame; }
	void SetAxControl(HWND hAxWnd);

	CChildFrame* GetChildFrame() const				{ return m_pChildFrame; }
	CWindow GetTabView() const;
	CWindow GetAxControl() const					{ return m_wndAxControl; }
	DWORD GetAxThreadID() const						{ return m_dwAxThreadID; }
	int GetCoreType() const							{ return m_iCoreType; }
	int GetCoreTypeSource() const					{ return m_iCoreTypeSource; }
	void SetCoreType(int iCore)						{ m_iCoreType = iCore; }
	void SetCoreTypeSource(int iSrc)				{ m_iCoreTypeSource = iSrc; }
	bool SwitchCoreUsed() const						{ return m_bSwitchCoreUsed; }
	void SetSwitchCoreUsed(bool bFlag)				{ m_bSwitchCoreUsed = bFlag; }

	PageType GetPageType() const		{ return m_ePageType; }
	void SetPageType(PageType eType)	{ m_ePageType = eType; }
	SpecialPageType GetSpecialPageType() const		{ return m_eSpecialPageType; }
	StartPageType GetStartPageType() const			{ return m_eStartPageType; }
	void SetStartPageType(StartPageType eStartPage)	{ m_eStartPageType = eStartPage; }

	bool IsCloseBorder() const;				// ����Ƿ�Ϊ�رյı߽磨�����ʣ���һ����ǩ�ˣ��Ͳ������ٹرգ�
	bool CanBeOverwrite() const;			// ��ǰ���ҳ���Ƿ���Ա����ǣ����㿪�ղص�����ʱ�ڵ�ǰ���ҳ��򿪣�

	LPCTSTR GetText() const;
	LPCTSTR GetURLText() const			{ return m_sURL.c_str(); }
	LPCTSTR GetDatabaseURL() const		{ return m_sDatabaseURL.c_str(); }
	size_t GetURLTextLength() const		{ return m_sURL.length(); }
	size_t GetDatabaseURLLength() const		{ return m_sDatabaseURL.length(); }
	void SetText(LPCTSTR lpszText);
	void SetURLText(LPCTSTR lpszURL);
	void SetDatabaseURL(LPCTSTR lpszURL)		{ m_sDatabaseURL = lpszURL; }
	int GetPageWeight() const			{ return m_iPageWeight; }
	void SetPageWeight(int iWeight)		{ m_iPageWeight = iWeight; }
	bool IsPageViewSource() const;

	void SetIcon(HICON hIcon);
	bool LoadIconFromURL(LPCTSTR lpszURL, HWND hCallbackWnd);			// ����ֵ��ʾicon�����ı�

	void SetProgress(int iProgress);
	int GetProgress() const				{ return m_iLoadingProgress; }

	void SetAdditinalData(LPCTSTR lpszAdditionalData, int iType);
	LPCTSTR GetAdditinalData() const			{ return m_sAdditionalData.c_str(); }
	int GetAdditionalType() const				{ return m_iAdditionalDataType; }

	void SetStatusText(LPCTSTR lpszText)	{ m_sStatus = lpszText; }
	LPCTSTR GetStatusText() const			{ return m_sStatus.c_str(); }
	void SetStatusIcon(int iIconIndex)		{ m_iStatusIcon = iIconIndex; }
	int GetStatusIcon() const				{ return m_iStatusIcon; }

	void SetMainToolbarStatus(int iType, bool bEnabled);
	bool GetMainToolbarBackStatus() const			{ return m_bIsBackEnabled; }
	bool GetMainToolbarForwardStatus() const		{ return m_bIsForwardEnabled; }

	void SetSearchBarStatus(LPCTSTR lpszSearchString, short iHiLight, short iSearchEngine, bool bForce = false);
	LPCTSTR GetSearchBarString() const	{ return m_strSearchString.c_str(); }
	bool GetSearchBarHiLight() const	{ return m_bSearchHilight; }
	short GetSearchBarEngine() const	{ return m_iSearchEngine; }

	LPCTSTR GetFindString() const		{ return m_strFindText.c_str(); }
	void SetFindString(LPCTSTR str)		{ m_strFindText = str; }
	bool GetFindHiLight() const			{ return m_bFindHiLight; }
	void SetFindHiLight(bool bFindHiLight)	{ m_bFindHiLight = bFindHiLight; }
	bool GetFindCaseSensitive() const		{ return m_bFindCaseSensitive; }
	void SetFindCaseSensitive(bool bCase)	{ m_bFindCaseSensitive = bCase; }
	int GetTotalFind() const			{ return m_iTotalFind; }
	void SetTotalFind(int iTotalFind)	{ m_iTotalFind = iTotalFind; }
	int GetCurrentFind() const			{ return m_iCurrentFind; }
	void SetCurrentFind(int iCurrentFind)	{ m_iCurrentFind = iCurrentFind; }
	bool IsFinding() const				{ return m_bIsFinding; }
	void SetIsFinding(bool bIsFinding)	{ m_bIsFinding = bIsFinding; }

	void SetFontSize(int iSize)			{ m_iFontSize = iSize; }
	int GetFontSize() const				{ return m_iFontSize; }
	void SetZoom(int iZoom)				{ m_iZoom = iZoom <= 20 ? 20 : (iZoom >= 400 ? 400 : iZoom); }
	int GetZoom() const					{ return m_iZoom; }
	DWORD GetInitZoomValue();

	int SetResourceStringList(LPCTSTR szList);			// ����ֵΪ��һ���ַ������������
	int GetResourceCount(int iRes)		{ return m_strResourceTitle[iRes].size(); }
	StrArray& GetResourceStringList(int iRes)		{ return m_strResourceTitle[iRes]; }

	StrArray& GetBlockedPopups()			{ return m_strBlockedPopups; }
	StrArray& GetBlockedFloatingElements()  { return m_strBlockedFloatingElements; }
	int GetBlockedCount()					{ return m_strBlockedPopups.size() + m_strBlockedFloatingElements.size(); }

	void SetNewWidth();
	void SetNewWidthImmediately();
	int GetAniPos() { return m_iXPosAni; }
	void SetPos(int iPos);
	void SetPosImmediately(int iPos);
	void UpdateAnimateXPos();
	void UpdateAnimateWidth();
	//void FinishAnimation(CTuotuoTabCtrl *pOPTab);

	size_t GetIndex() const				{ return m_nIndex; }
	void SetIndex(size_t nIndex)		{ m_nIndex = nIndex; }


	void IncExceptionCount(DWORD dwCode)	{ m_iExceptionCount ++; m_dwLastExceptionCode = dwCode; }
	int GetExceptionCount() const			{ return m_iExceptionCount; }
	DWORD GetLastExceptionCode() const		{ return m_dwLastExceptionCode; }
	DWORD GetRequestNum() const             { return m_dwRequestNum; }

	void SetAutofillTip(bool bFlag)			{ m_bDisplayAutofillTip = bFlag; }
	bool IsAutofillTipShown() const			{ return m_bDisplayAutofillTip; }

	void SetSecure(bool bSecure)			{ m_bIsSecure = bSecure; }
	bool GetSecure() const					{ return m_bIsSecure; }

	bool IsNoClose() const					{ return m_bNoClose; }

public:

	CCategoryItem *m_pCategory;

private:

	bool m_bNoClose;

	CTuotuoTabCtrl *m_pTabCtr;
	CChildFrame *m_pChildFrame;
	CWindow m_wndAxControl;
	DWORD m_dwAxThreadID;
	int m_iCoreType;
	int m_iCoreTypeSource;			// �жϺ˵���Դ��0=ϵͳ������1=�û�����
	bool m_bSwitchCoreUsed;			// �û��Ƿ��ֹ��л�����

	int m_nIndex;

	PageType m_ePageType;
	SpecialPageType m_eSpecialPageType;
	StartPageType m_eStartPageType;

	std::tstring m_sURL;
	std::tstring m_sDatabaseURL;		// ��¼��database�е�url����ҳ������ת���������m_sURL�ǲ�ͬ��
	std::tstring m_sText;
	std::tstring m_sStatus;
	std::tstring m_sAdditionalData;		// ��������ݣ�������referer������postdata
	int m_iAdditionalDataType;			// ������������ͣ�-1��ʾ�޶������ݣ�0��ʾreferer��1��ʾpostdata
	HICON m_hIcon;			// ҳ��ͼ��
	std::tstring m_strLastIconName;	// ��¼icon��Ӧ��filename
	int m_iPageWeight;			// ҳ�洴����Ȩ��

	bool m_bIsBackEnabled, m_bIsForwardEnabled;			// ǰ�������˰�ť�Ƿ����ʹ��

	int m_iStatusIcon;			// 0=loading, 1=load complete, 2=complete with error
	int m_iLoadingProgress;		// the current loading progress 0 ~ 1000

	std::tstring m_strSearchString;		// ������������
	bool m_bSearchHilight;			// �����Ƿ����
	short m_iSearchEngine;			// ����ʹ�õ���������

	std::tstring m_strFindText;			// ������������
	bool m_bFindHiLight;			// �������Ƿ����
	bool m_bFindCaseSensitive;		// �Ƿ����ִ�Сд
	int m_iTotalFind;				// �ܹ��ҵ������ָ�����-1Ϊ��û����
	int m_iCurrentFind;				// ��ǰ���ĸ�λ����
	bool m_bIsFinding;				// �Ƿ����ڲ��ң���childframe�Ѿ���axcontrol�����˲������󣬵��ǻ�δ���أ�

	int m_iFontSize;			// ����������С -2, -1, 0, 1, 2
	int m_iZoom;				// �����zoom���� 100%

	StrArray m_strResourceTitle[RESOURCE_TYPE_COUNT];		// ��ȡ��Դʹ��, 0��ʾ��Ƶ��1��ʾ��Ƶ

	StrArray m_strBlockedPopups;			// �����صĵ������ڵĵ�ַ
	StrArray m_strBlockedFloatingElements;	// �����ص�ҳ�渡��Ԫ��

	// for animation
	int m_iXPos;			// Ŀ��x����
	int m_iXPosAni;			// ��ǰ�����е�x����
	int m_iXPosAniFrame;	// ��ǰ���ڵڼ�֡
	int m_iXPosOld;			// ��������ǰ��x����

	int m_iWidth;
	int m_iWidthAni;
	int m_iWidthAniFrame;
	int m_iWidthOld;

	int m_iLoadingProgressFrame;
	DWORD m_dwLastUpdateLoadingProgressTime;

	// for pingback
	int m_iExceptionCount;			// ����exception�Ĵ���
	DWORD m_dwLastExceptionCode;	// ���һ��exception��code
	DWORD m_dwRequestNum ;          // Index
	bool m_bDisplayAutofillTip;		// �Ƿ���ʾС����

	bool m_bIsSecure;

public:

	HWND m_hThumbnailProxy;				// ��win7ʹ�õ���ʵ����ͼ��proxy

	friend class CTuotuoTabCtrl;
};
