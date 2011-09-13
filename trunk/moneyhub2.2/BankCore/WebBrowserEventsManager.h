#pragma once

class CAxControl;
class CCustomSite ;
class CFabricationHTMLWindow2 ;

class CWebBrowserEventsManager : public IDispatch
{
public :
	CWebBrowserEventsManager::CWebBrowserEventsManager(CAxControl *pAxControl);
	~CWebBrowserEventsManager() ;

public :
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv) ;
	STDMETHOD_(ULONG, AddRef)() ;
	STDMETHOD_(ULONG, Release)() ;
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) ;
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppInfo) ;
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) ;
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

	BOOL AdviseBrowserEvents(IWebBrowser2* pWebBrowser2) ;
	BOOL UnadviseBrowserEvents() ;
	BOOL SetCustomSite(CCustomSite* pDocHostUIHandler) ;
	void SetWindowOpenMode(BOOL bForceBackOpen) ;

	void SetHighLight(BOOL bToBeHighLight) ;
	void SetSearchString(LPTSTR lpszHighLight);

	void ResetWebString() ;
	void CheckNeedPrompt(LPCTSTR lpszUrl) ;
	void SetZoomPage(DWORD dwZoomPage){m_dwZoomPage=dwZoomPage;}
	DWORD SetPercentage(DWORD dwNew) {DWORD dwTemp = m_dwPercentage;m_dwPercentage = dwNew;return dwTemp;}

private :
	HRESULT OnTitleChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnStatuesChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnNewWindow2(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnNewWindow3(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnBeforeNavigate2(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnNavigateComplete2(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnDocumentComplete(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnProgressChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnWindowClosing(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnCommandStateChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnQuit(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnNavigateError(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT OnSetSecureLockIcon(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

	BOOL IsToppestWindowOfBrowser(IDispatch* pDispatch) ;
	BOOL IsToppestWindowOfBrowser2(IDispatch* pDispatch) ;
	BOOL IsValidHttpUrl(const BSTR) ;

	// �ۺ� OnNewWindow2 �� OnNewWindow3 �ĵ���
	// �� XP SP2 ϵͳ����߰汾�� OnNewWindow3��ԭ������Щ flash ������������ӣ��� OnNewWindow2 ���ò�����ȷ�� URL
	//
	// bstrUrl		- �¿����ڵ� URL
	// ppDisp		- �¿����ڵ� WebBrowser �� DISPATH ָ��
	// Cancel		- �Ƿ�ȡ���򿪴���
	HRESULT OnNewWindow(LPCTSTR lpszUrl, LPCTSTR lpszReferer, IDispatch **&ppDisp, VARIANT_BOOL *&Cancel);

	//////////////////////////////////////////////////////////////////////////
	// DWebBrowserEvents2
	void ProgressChange(long Progress,long ProgressMax);
private :

	CAxControl*                               m_pAxControl;
	IWebBrowser2*                             m_pWebBrowser2 ;
	CCustomSite*                              m_pCustomSite ;
	IDispatch*                                m_pDispDocument ;
	DWORD                                     m_dwCookie ;
	DWORD                                     m_dwPercentage ;
	DWORD                                     m_dwZoomPage ;
	DWORD                                     m_dwTotalFindString ;
	DWORD                                     m_dwCurrentFindString ;
	DWORD                                     m_dwHTMLWindowBaseAddr ;
	DWORD                                     m_dwNewestBeforeNavTick ;
	DWORD                                     m_dwLastSubmitTickCount ;
	BOOL                                      m_bForceBackOpen ;
	BOOL                                      m_bShowTitle ;
	BOOL                                      m_bInsertTitle ;
	BOOL                                      m_bPostDataInTopFrame ;
	BOOL                                      m_bPostDataInAnyFrame ;
	BOOL                                      m_bFormCheck ;
	long                                      m_lOldStatus ;
	CString                                   m_cstrFindString ;

public:

	bool m_error;
	HGLOBAL m_hTempWriteHTMLData;
	DWORD m_dwAxUserActionTick ;

	CString m_cstrOriUrl ;
	CString m_cstrLastUrl ;
	CString m_cstrConnectedUrl ;
	CString m_cstrErrUrl ;

	CString m_cstrAnchorText ;
	CString m_cstrTitle ;
	CString m_cstrPostData ;
public:
	std::wstring  m_adv;
	std::string GetFilterFile();
	std::string ws2s(const std::wstring& ws);
	std::wstring Replace( const std::wstring& orignStr, const std::wstring& oldStr, const std::wstring& newStr);
};
