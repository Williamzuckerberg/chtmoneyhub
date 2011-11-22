#include "stdafx.h"
#include <fstream>
#include "ConvertBase.h"
#include "Util.h"
#include "../Utils/ListManager/URLList.h"
#include "../Utils/Config/HostConfig.h"
#include "AxControl.h"
#include "CustomSite.h"
#include "WebBrowserEventsManager.h"
#include "DownloadManagerImpl.h"
#include "../Utils/HardwareID/genhwid.h"
#include "../BankUI/Util/Util.h"
#include "../BankUI/UIControl/CoolMessageBox.h"
#include "../Utils/UserBehavior/UserBehavior.h"
#include "..\BankUI\UIControl\SelectMonthDlg.h"
#include "GetBill/BillUrlManager.h"
#include "ExternalDispatchImpl.h"
#include "BankData\BkInfoDownload.h"
#include "MyError.h"
#include "../BankUI/UIControl/MonthSelectDlg.h"

#define MODE_UNDEFINED                     ((DWORD)0)
#define MODE_HANDINPUT                     ((DWORD)1)
#define MODE_SELECTDROPLIST                ((DWORD)2)
#define MODE_OTHERS                        ((DWORD)500)

#define  BEGIN_GET_BILL_STEPcmbchina	2
#define  BEGIN_GET_BILL_STEPecitic	6



BOOL CWebBrowserEventsManager::AdviseBrowserEvents(IWebBrowser2* pWebBrowser2)
{
	m_pWebBrowser2 = pWebBrowser2 ;
	HRESULT hr = AtlAdvise(m_pWebBrowser2, (IUnknown*)this, DIID_DWebBrowserEvents2, &m_dwCookie);
	return SUCCEEDED(hr) ;
}

BOOL CWebBrowserEventsManager::UnadviseBrowserEvents()
{
	if (!m_pWebBrowser2 || !m_dwCookie)
	{
		ATLASSERT(FALSE) ;
		return FALSE ;
	}

	HRESULT hr = AtlUnadvise(m_pWebBrowser2, DIID_DWebBrowserEvents2, m_dwCookie) ;
	return SUCCEEDED(hr) ;
}

CWebBrowserEventsManager::CWebBrowserEventsManager(CAxControl *pAxControl): m_pAxControl(pAxControl)
{
	m_notifyWnd = 0;
	m_error = false;
	m_pBillData = NULL;
	m_isGetBill = false;
	m_step = 1;
	m_iCancelState = 0;
	m_bCanClose = true;

	m_dwCookie = 0 ;
	m_pCustomSite = NULL ;
	m_dwPercentage = 0 ;
	m_dwZoomPage = 100 ;
	m_bForceBackOpen = FALSE ;
	m_bShowTitle = FALSE ;
	m_bInsertTitle = FALSE ;
	m_cstrFindString = L"" ;
	m_dwTotalFindString = 0 ;
	m_dwCurrentFindString = -1 ;
	m_dwLastSubmitTickCount = 0;
	m_lOldStatus = -1 ;
	m_pDispDocument = NULL ;
	m_pWebBrowser2 = NULL ;
	m_dwNewestBeforeNavTick = GetTickCount() ;
	m_hTempWriteHTMLData = NULL;
	m_cstrOriUrl = L"" ;
	m_cstrLastUrl = L"" ;
	m_cstrConnectedUrl = L"" ;
	m_dwAxUserActionTick = GetTickCount() - 200 ;
	m_bPostDataInTopFrame = FALSE ;
	m_bPostDataInAnyFrame = FALSE ;
	m_bFormCheck = FALSE ;
	m_adv = CHostContainer::GetInstance()->GetHostName(kAdv);

}

CWebBrowserEventsManager::~CWebBrowserEventsManager()
{
	if (m_hTempWriteHTMLData)
		::GlobalFree(m_hTempWriteHTMLData);
}

ULONG CWebBrowserEventsManager::AddRef()
{
	return 1;
}

ULONG CWebBrowserEventsManager::Release()
{
	return 1;
}


HRESULT CWebBrowserEventsManager::QueryInterface(const IID &riid, void **ppv)
{
	if (!ppv)
	{
		return E_POINTER ;
	}

	if (IID_IUnknown == riid)
	{
		AddRef();
		*ppv = (IUnknown*)this ;
		return S_OK ;
	}
	else if (IID_IDispatch == riid)
	{
		AddRef();
		*ppv = (IDispatch*)this ;
		return S_OK ;
	}

	return E_NOTIMPL ;
}


BOOL CWebBrowserEventsManager::SetCustomSite(CCustomSite* pCustomSite)
{
	if (!pCustomSite)
	{
		return FALSE ;
	}
	m_pCustomSite = pCustomSite ;
	return TRUE ;
}

void CWebBrowserEventsManager::SetWindowOpenMode(BOOL bForceBackOpen)
{
	m_bForceBackOpen = bForceBackOpen ;
}

HRESULT CWebBrowserEventsManager::GetTypeInfoCount(UINT* pctinfo)
{
	return E_NOTIMPL ;
}

HRESULT CWebBrowserEventsManager::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppInfo)
{
	return E_NOTIMPL ;
}

HRESULT CWebBrowserEventsManager::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
	return E_NOTIMPL ;
}

HRESULT CWebBrowserEventsManager::Invoke(DISPID dispIdMember, const IID &riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	switch(dispIdMember)
	{
	case DISPID_TITLECHANGE :
		return OnTitleChange(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_STATUSTEXTCHANGE :
		return OnStatuesChange(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_NEWWINDOW2 :
		return OnNewWindow2(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_NEWWINDOW3:
		return OnNewWindow3(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);

	case DISPID_BEFORENAVIGATE2 :
		return OnBeforeNavigate2(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_NAVIGATECOMPLETE2 :
		return OnNavigateComplete2(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_DOCUMENTCOMPLETE :
		return OnDocumentComplete(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_PROGRESSCHANGE :
		return OnProgressChange(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_WINDOWCLOSING :
		return OnWindowClosing(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_COMMANDSTATECHANGE :
		return OnCommandStateChange(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_NAVIGATEERROR :
		return OnNavigateError(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;

	case DISPID_SETSECURELOCKICON :
		return OnSetSecureLockIcon(riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr) ;
	}
	
	return E_FAIL ;
}


HRESULT CWebBrowserEventsManager::OnCommandStateChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if(m_isGetBill)
		if(m_step > 1)
			return S_OK;

	if (pDispParams->cArgs != 2)
	{
		return E_INVALIDARG ;
	}

	if (CSC_NAVIGATEBACK == pDispParams->rgvarg[1].lVal)
	{
		::PostMessage(m_pAxControl->m_hChildFrame, WM_ITEM_SET_MAIN_TOOLBAR, MAKEWPARAM(0, pDispParams->rgvarg[0].boolVal), 0);
	}
	else if (CSC_NAVIGATEFORWARD == pDispParams->rgvarg[1].lVal)
	{
		::PostMessage(m_pAxControl->m_hChildFrame, WM_ITEM_SET_MAIN_TOOLBAR, MAKEWPARAM(1, pDispParams->rgvarg[0].boolVal), 0);
	}

	return S_OK ;
}

HRESULT CWebBrowserEventsManager::OnWindowClosing(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	return S_OK ;
}

HRESULT CWebBrowserEventsManager::OnStatuesChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	// ����ǵ����˵�ҳ����ֹ���½�����Ϣ�ĸı�
	if(m_isGetBill)
		if(m_step > 1)
			return S_OK;

	if (pDispParams->cArgs < 1)
	{
		return E_FAIL ;
	}

	// ���˵�һЩδ֪��Ϣ����ʾ��״̬����
	if(pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[0].bstrVal != NULL)
	{
		if (!wcsncmp(L"javascript:", pDispParams->rgvarg[0].bstrVal, 11)   ||
			!wcsncmp(L"vbscript:", pDispParams->rgvarg[0].bstrVal, 9)      ||
			!wcsncmp(L"mailto:", pDispParams->rgvarg[0].bstrVal, 7)        ||
			!wcsncmp(L"about:", pDispParams->rgvarg[0].bstrVal, 6)         ||
			!wcsncmp(L"res:", pDispParams->rgvarg[0].bstrVal, 4)		  ||
			!wcsncmp(L"http:", pDispParams->rgvarg[0].bstrVal, 5)         ||
			!wcsncmp(L"file:", pDispParams->rgvarg[0].bstrVal, 5))
		{
			return S_OK ;
		}
	}

	wstring sinfo(pDispParams->rgvarg[0].bstrVal);
	size_t n = sinfo.find(L"http");
	if (n != std::wstring::npos)
	{
		sinfo = sinfo.substr(0,  n - 1);
	}

	n = sinfo.find(L"file");
	if (n != std::wstring::npos)
	{
		sinfo = sinfo.substr(0,  n - 1);
	}
	if(sinfo != L"")
		::SendMessage(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_SET_STATUS, (LPARAM)sinfo.c_str());

	return S_OK ;
}

// OnNewWindow3 �� XP SP2 ����߰汾���У��� OnNewWindow2 ����ĳЩ flash �����������ò�����Ӧ�� URL
// ��ˣ������ XP SP2 ����߰汾�������� OnNewWindow3.
//
HRESULT CWebBrowserEventsManager::OnNewWindow2(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	return S_FALSE;
}

HRESULT CWebBrowserEventsManager::OnNewWindow3(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (pDispParams->cArgs == 5)
		return OnNewWindow(pDispParams->rgvarg[0].bstrVal, pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[4].ppdispVal, pDispParams->rgvarg[3].pboolVal);
	return S_FALSE;
}

// wanyong 2009-1-23
// �������ڵ������е�С����
// �ο� http://www.archivum.info/microsoft.public.inetsdk.programming.webbrowser_ctl/2006-04/msg00015.html
//         http://www.archivum.info/microsoft.public.inetsdk.programming.webbrowser_ctl/2006-04/msg00016.html
//
// msdn��Ҳ�����ˣ����£�����ȷ���������ȴ��������ص�û��navigate��webbrowserʵ�����ɣ�Ȼ�������̻߳����̵�marshal
// ------------------------------------------------
// From DWebBrowserEvents2::NewWindow3 Event
// The application that processes this notification can respond in one of three ways. 
// 
// * Create a new, hidden, nonnavigated WebBrowser or InternetExplorer object that is returned in ppDisp. 
//			Upon return from this event, the object that raised this event will then configure and navigate (including a BeforeNavigate2 event) 
//			the new object to the target location.
// * Cancel the navigation by setting Cancel to VARIANT_TRUE.
// * Do nothing and do not set ppDisp to any value. This will cause the object that raised the event to create a new InternetExplorer 
//			object to handle the navigation.
// ------------------------------------------------
//
std::wstring CWebBrowserEventsManager::Replace( const std::wstring& orignStr, const std::wstring& oldStr, const std::wstring& newStr)
{   
	size_t pos = 0;
	wstring tempStr = orignStr;
	wstring::size_type newStrLen = newStr.length();
	wstring::size_type oldStrLen = oldStr.length();
	while(true)
	{   
		pos = tempStr.find(oldStr, pos);
		if (pos == wstring::npos) 
			break;   
		tempStr.replace(pos, oldStrLen, newStr);
		pos += newStrLen;  
	}  
	return tempStr;   
} 
HRESULT CWebBrowserEventsManager::OnNewWindow(LPCTSTR lpszUrl, LPCTSTR lpszReferer, IDispatch **&ppDisp, VARIANT_BOOL *&Cancel)
{
	// ����ǽű������ڱ�������ִ��
	if (GetKindOfNavigateUrl(lpszUrl) == SCRIPT_SCHEME_KEY)
	{
		return S_OK;
	}

	if (!::IsWindow(m_pAxControl->m_hChildFrame))
	{
		*Cancel = TRUE;
		return S_OK;
	}
	if(_tcsnicmp(lpszUrl, _T("about:blank"), 11) == 0)
	{
		if(lpszReferer != NULL)//���ڲ���windows.open(url,"_blank")���ִ�����ҳ�Ĵ��������й�����
			lpszUrl = lpszReferer;
	}

	//����׵������Լ���ҳ��		
	std::wstring  url(lpszUrl);
	size_t dpos = url.find(m_adv);

	if(dpos != std::wstring::npos)
	{
		size_t spos = url.find(L"http", dpos);
		if(spos != std::wstring::npos)
		{
			std::wstring suburl = url.substr(spos);

			std::wstring turl = Replace(suburl, L"%3A", L":");
			url = Replace(turl, L"%2F", L"/");
			lpszUrl = url.c_str();
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// 
	std::wstring strDomain = ExtractTopDomain(lpszUrl);
	const CWebsiteData *pData = CURLList::GetInstance()->GetData(L"", lpszUrl);

	if (pData == NULL)
	{
		if (_tcsnicmp(lpszUrl, _T("http"), 4) == 0)
		{
			*Cancel = TRUE;
			CListManager::NavigateWebPage(lpszUrl);

/*
			IWebBrowser2* pWebBrowser2 = NULL;
			CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (void**)&pWebBrowser2);
			if (pWebBrowser2)
			{
				pWebBrowser2->get_Application(ppDisp);
				pWebBrowser2->Release();
			}
*/

			return S_OK;
		}
	}


	//////////////////////////////////////////////////////////////////////////

	bool bOpenBackground = false;
	// Control �����ȼ���ߣ�������¾�һ���ں�̨��
	if (GetAsyncKeyState(VK_CONTROL) < 0 || m_bForceBackOpen)
		bOpenBackground = true;
	// Shift ���ȼ��θ�
	else if (GetAsyncKeyState(VK_SHIFT) < 0)
		bOpenBackground = false;


	//////////////////////////////////////////////////////////////////////////

	HWND hChildFrame = NULL;
	
	enum TopPageStruct tps = (enum TopPageStruct)CListManager::GetDefaultPageIndex(lpszUrl);
	if (tps != kPageInvalid)
	{
		HWND hFrameWnd = ::GetRootWindow(m_pAxControl->m_hChildFrame);
		SwitchTopPage(hFrameWnd, tps);

		*Cancel = TRUE;
		return S_OK;
	}
	else if (pData == NULL)
	{
		if (_tcsnicmp(lpszUrl, _T("about:blank"), 11) != 0)
		{
			*Cancel = TRUE;
			CListManager::NavigateWebPage(lpszUrl);

			return S_OK;
		}
	}

	HWND hAxControl = NULL;
	if(m_isGetBill)
	{
		bOpenBackground = true;
		if(m_step > 1)
			if (_tcsnicmp(lpszUrl, _T("about:blank"), 11) == 0)
			{
				*Cancel = TRUE;
				return S_OK;
			}
	}
	if (hChildFrame == NULL)
	{
		if (bOpenBackground)
		{
			hChildFrame = (HWND)CreateNewPage_3_NewWindow(m_pAxControl->m_hChildFrame, lpszUrl, FALSE, m_pAxControl->m_hChildFrame, m_isGetBill);
			m_bForceBackOpen = FALSE;
		}
		else
			hChildFrame = (HWND)CreateNewPage_3_NewWindow(m_pAxControl->m_hChildFrame, lpszUrl, TRUE, m_pAxControl->m_hChildFrame, m_isGetBill);
	}

	if (hChildFrame == NULL)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_STATE, CRecordProgram::GetInstance()->GetRecordInfo(L"OnNewWindow-CreateNewPage_3_ExistWindow-2ʧ��:%s",lpszUrl));
		*Cancel = TRUE;
		return S_OK;
	}


	for (int i = 0; i < 400; ++i)
	{
		hAxControl = (HWND)::SendMessage(hChildFrame, WM_ITEM_GET_AX_CONTROL_WND, 0, 0);
		if (hAxControl)
			break ;
		Sleep(10) ;
	}


	bool bSuccess = false;
	if (hAxControl != NULL)
	{
		DWORD dwProcessIDDescendant = 0;
		GetWindowThreadProcessId(hAxControl, &dwProcessIDDescendant) ;
		if (dwProcessIDDescendant == GetCurrentProcessId())
		{		
			LPARAM lParam = 0;
			WPARAM isGetBill = 0;

			if(m_isGetBill)
			{
				lParam = (LPARAM)m_pBillData;
				isGetBill = 2 *100 + m_step;
			}
			// ����ǵ����˵�ҳ����ô�������˵������ݺͽ��е��Ĳ���֪ͨ������ҳ��
			IStream* pStream = (IStream*)::SendMessageBlock(hAxControl, WM_AX_GET_WEBBROWSER2_CROSS_THREAD, isGetBill, lParam) ;
			if (pStream)
			{
				CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_STATE, CRecordProgram::GetInstance()->GetRecordInfo(L"OnNewWindow-���ҳ��%s�ɹ�", lpszUrl));
				CComPtr<IWebBrowser2> spWebBrowser2;
				if (SUCCEEDED(CoGetInterfaceAndReleaseStream(pStream, IID_IWebBrowser2, (void**)&spWebBrowser2)) && spWebBrowser2)
				{	
					spWebBrowser2->put_RegisterAsDropTarget(VARIANT_FALSE);

					// ��������js��Ҫʶ�𴰿���
					if (_tcsstr(lpszReferer, _T("ebank.cmbchina.com")) != NULL)
						spWebBrowser2->put_RegisterAsBrowser(VARIANT_TRUE);

					if (SUCCEEDED(spWebBrowser2->get_Application(ppDisp)) && ppDisp)
						return S_OK;
					else
						CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_STATE, CRecordProgram::GetInstance()->GetRecordInfo(L"OnNewWindow-get_Applicationʧ��:%s",lpszUrl));
				}
				else
				{
					CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_STATE, CRecordProgram::GetInstance()->GetRecordInfo(L"OnNewWindow-CoGetInterfaceAndReleaseStreamʧ��:%s",lpszUrl));
				}
			}
		}
	}
	if (!bSuccess)
	{
		*Cancel = TRUE;
	}

	return S_OK;
}

HRESULT CWebBrowserEventsManager::OnTitleChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (pDispParams->cArgs < 1)
		return E_FAIL ;
	// ����ǵ����˵�ҳ����ֹ���Ľ������ʾ
	if(m_isGetBill)
		if(m_step > 1)
			return S_OK;

	if (!m_bShowTitle && pDispParams->rgvarg[0].bstrVal[0])
	{
		::SendMessage(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_SET_TAB_TEXT, (LPARAM)pDispParams->rgvarg[0].bstrVal);
		m_cstrTitle = pDispParams->rgvarg[0].bstrVal ;

		if (0 == m_bInsertTitle && m_cstrConnectedUrl.GetLength() > 0)
		{
			CString cstr = m_cstrConnectedUrl ;
			cstr += L"\n\n" ;
			cstr += pDispParams->rgvarg[0].bstrVal ;
		}
		if (m_cstrConnectedUrl.GetLength() > 0)
			if (m_cstrConnectedUrl.Compare(pDispParams->rgvarg[0].bstrVal))
				if (wcsncmp(pDispParams->rgvarg[0].bstrVal, L"http", 4))
					m_bInsertTitle = TRUE ;
	}
	return S_OK ;
}

HRESULT CWebBrowserEventsManager::OnBeforeNavigate2(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	m_error = false;
	if(pDispParams->cArgs < 7)
	{
		return E_FAIL;
	}
	// �ڸô��������˵�ҳ�濪ʼ���к��м�ؼ��������ʼ����
	if(m_isGetBill)
	{
		LPURLLIST pUrlList = CBillUrlManager::GetInstance()->GetUrlMap(m_pBillData->aid, m_pBillData->type);
		if(pUrlList == NULL)
			return S_OK;
		map<int, wstring>* purlmap =  &(pUrlList->url);
		wstring strSour = pDispParams->rgvarg[5].pvarVal->bstrVal;
		std::map<int, wstring>::iterator ite= purlmap->begin();

		if( CBillUrlManager::GetInstance()->Getmode(m_pBillData->aid) )
		{
			for(;ite != purlmap->end(); ite ++)
			{
					wstring url = ite->second;
					CString   strtmpqq;
       				CString   strtmp = (CString)pDispParams->rgvarg[5].pvarVal->bstrVal;
					strtmpqq = strtmp.Mid(0,url.size());
					if( find(pUrlList->m_beginstep.begin(), pUrlList->m_beginstep.end(), ite->first) != pUrlList->m_beginstep.end())//
						strtmpqq = strtmp;
					if(strtmpqq == url.c_str() && m_stepTime.size() == 0 && ite->first == 2)
					{
     					m_stepTime.push_back(ite->first);
						NotifyGettingBill();
						ShowNotifyWnd(dLoginDlg); //��ʾ���ڵ�½
					}

			}

		}
		else
		{
			for(;ite != purlmap->end(); ite ++)
			{
				wstring url = ite->second;

				if(url.size() > 0 && wcsncmp(url.c_str(), strSour.c_str(), url.size()) == 0)
				{
					CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_GET_BILL, CRecordProgram::GetInstance()->GetRecordInfo(L"�����˵�����ؼ�ҳ��%d:%s:%s", m_step, url.c_str(), strSour.c_str()));
					if(find(m_stepTime.begin(), m_stepTime.end(), ite->first) == m_stepTime.end())//ֻ�����һ�γ��ֵ�����
					{
						m_stepTime.push_back(ite->first);
						if(ite->first > m_step)
						{
							m_step = ite->first;//��ʼ����Ĵ���������
						}

						if( find(pUrlList->m_beginstep.begin(), pUrlList->m_beginstep.end(), m_step) != pUrlList->m_beginstep.end())//����ڽ��������
						{
							NotifyGettingBill();
							ShowNotifyWnd(dLoginDlg);//��ʾ���ڵ�½
						}
					}						
				}
			}
		}

	}
	//////////////////////////////////////////////////////////////////////////
	// Download Catch

	BOOL bClosed = FALSE;
	IDispatch* pDispatch = pDispParams->rgvarg[6].pdispVal;
	if (pDispatch && m_pWebBrowser2)
	{
		CComPtr<IUnknown> pUnk;
		CComPtr<IDispatch> pWBDisp;

		HRESULT hr = m_pWebBrowser2->QueryInterface(IID_IUnknown, (void**)&pUnk);
		HRESULT hr2 = pUnk->QueryInterface(IID_IDispatch, (void**)&pWBDisp);
		if (SUCCEEDED(hr) && SUCCEEDED(hr2) && pDispatch && pDispatch == pWBDisp)
		{
			BSTR bstrUrl;
			m_pWebBrowser2->get_LocationURL(&bstrUrl);
			if (SysStringLen(bstrUrl) == 0)
				bClosed = TRUE;
			SysFreeString(bstrUrl);
		}
	}

	HWND hCloseWnd = bClosed ? m_pAxControl->m_hChildFrame : NULL;
	CAsyncDownloadHelper cHelper(m_pAxControl->m_hChildFrame, hCloseWnd);
	if (SUCCEEDED(cHelper.BeforeNavigate2(pDispParams->rgvarg[6].pdispVal, 
										  pDispParams->rgvarg[5].pvarVal, 
										  pDispParams->rgvarg[4].pvarVal, 
										  pDispParams->rgvarg[3].pvarVal, 
										  pDispParams->rgvarg[2].pvarVal, 
										  pDispParams->rgvarg[1].pvarVal, 
										  pDispParams->rgvarg[0].pboolVal)))
	{
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_STATE, CRecordProgram::GetInstance()->GetRecordInfo(L"DownloadHelper: %s-��ʼ����", pDispParams->rgvarg[5].pvarVal->bstrVal));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	CString strUrl(pDispParams->rgvarg[5].pvarVal->bstrVal);
	const CWebsiteData* pData = NULL;

	if((wcsncmp(L"https:", pDispParams->rgvarg[5].pvarVal->bstrVal, 6) == 0))
	{
		USES_CONVERSION;

		pData = CURLList::GetInstance()->GetData(L"", strUrl);
		if(NULL != pData)
		{
			//�Է���httpsҳ��Ž����ղ�
			if(wcsncmp(L"https:", pDispParams->rgvarg[5].pvarVal->bstrVal, 6) == 0)
			{
				USES_CONVERSION;
				std::string appid = CFavBankOperator::GetBankIDOrBankName(W2A(pData->GetID()), false);
				if(appid != "")
				{
					if( !CBankData::GetInstance()->IsFavExist(appid))
					{
						CStringW msg;
						msg.Format(_T("��⵽%sû���ղأ���Ҫ���ڽ����ղ���"), pData->GetName());
						DWORD dwVersion = GetVersion();
						int	res;
						// win7���ڵ��Ӵ��ڵĴ����̱߳�ӵ���󣬸�����Ҳ������Ӧ��״�������������ﵥ�������߳�ȥ��ʾ����������ô����߳���ʾѯ�ʽ���Ĳ���
						if((DWORD)(LOBYTE(LOWORD(dwVersion))) > 5)
						{
							DWORD dw;
							MsgData md;
							md.type = 1;
							md.info = msg.GetString();
							md.wtype = MB_OKCANCEL;
							HANDLE hThread = CreateThread(NULL, 0, _threadShowWaitDLG, (LPVOID)&md, 0, &dw);

							WaitForSingleObject(hThread, INFINITE);
							//��ȡ�û�ѡ��Ľ��
							res = md.res;
						}
						else
							res = ::SendMessage(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_ASK_SAVE_FAV, (LPARAM)msg.GetString());
						if(res == IDOK)
						{							
							CNotifyFavProgress::GetInstance()->AddFav(appid);//����js��ʼ��װ;
							wstring smsg = A2W(appid.c_str()); 
							msg.Format(_T("%s��������%s�ؼ������Ժ�......"), smsg.c_str(), pData->GetName());
							//int res = ::SendMessageW(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_TEST_SAVE_FAV, (LPARAM)msg.GetString());

							if((DWORD)(LOBYTE(LOWORD(dwVersion))) > 5)
							{
								DWORD dw;
								MsgData md;
								md.type = 2;
								md.info = msg.GetString();
								md.wtype = MB_OKCANCEL;
								HANDLE hThread = CreateThread(NULL, 0, _threadShowWaitDLG, (LPVOID)&md, 0, &dw);

								WaitForSingleObject(hThread, INFINITE);
							}
							else
								::SendMessageW(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_TEST_SAVE_FAV, (LPARAM)msg.GetString());
						}
					}
				}
			}
		}
	}

	if (IsToppestWindowOfBrowser2(pDispParams->rgvarg[6].pdispVal))
	{
		if (!wcsncmp(L"javascript:", pDispParams->rgvarg[5].pvarVal->bstrVal, 11)   ||
			!wcsncmp(L"vbscript:", pDispParams->rgvarg[5].pvarVal->bstrVal, 9)      ||
			!wcsncmp(L"mailto:", pDispParams->rgvarg[5].pvarVal->bstrVal, 7)        ||
			!wcsncmp(L"about:", pDispParams->rgvarg[5].pvarVal->bstrVal, 6)         ||
			!wcsncmp(L"res:", pDispParams->rgvarg[5].pvarVal->bstrVal, 4))
		{
			return S_OK ;
		}


		if((wcsncmp(L"https:", pDispParams->rgvarg[5].pvarVal->bstrVal, 6) == 0) || 
			(wcsncmp(L"http:", pDispParams->rgvarg[5].pvarVal->bstrVal, 5) == 0) || 
			(wcsncmp(L"file:", pDispParams->rgvarg[5].pvarVal->bstrVal, 5) == 0))
		{
			USES_CONVERSION;

			//�����Լ��Ĺ��ҳ����ת��
			std::wstring  url(pDispParams->rgvarg[5].pvarVal->bstrVal);
			size_t dpos = url.find(m_adv);


			if(dpos != std::wstring::npos)
			{
				size_t spos = url.find(L"http", dpos);
				if(spos != std::wstring::npos)
				{
					std::wstring suburl = url.substr(spos);
					std::wstring turl = Replace(suburl, L"%3A", L":");
					url = Replace(turl, L"%2F", L"/");
					strUrl = url.c_str();
				}
			}
		}
		if(NULL == pData)
			pData = CURLList::GetInstance()->GetData(L"", strUrl);
		if(NULL == pData)
		{
			*pDispParams->rgvarg[0].pboolVal = VARIANT_TRUE;
			CListManager::NavigateWebPage(strUrl);

			::PostMessage(m_pAxControl->m_hChildFrame, WM_ITEM_AUTOCLOSE, 0, 0);

			return E_FAIL;
		}

		m_cstrLastUrl = pDispParams->rgvarg[5].pvarVal->bstrVal ;
		::SendMessage(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_SET_TAB_URL, (LPARAM)pDispParams->rgvarg[5].pvarVal->bstrVal);
	}

	return S_OK ;
}

HRESULT CWebBrowserEventsManager::OnNavigateComplete2(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (pDispParams->cArgs < 2)
	{
		return E_FAIL;
	}

	//������Ҫ�������ҳ��Ϊ�����˵�ҳ�棬��ô��ֹ�޸Ľ����ϵ���ʾ��Ϣ
	if(m_isGetBill)
		if(m_step > 1)
			return S_OK;

	if (IsToppestWindowOfBrowser2(pDispParams->rgvarg[1].pdispVal))
	{
		if(FileterNormalUrl( pDispParams->rgvarg[0].pvarVal->bstrVal))
			return S_OK ;

		if (0 == m_cstrConnectedUrl.GetLength() || -1 == m_cstrConnectedUrl.Find(L"http://"))
		{
			CString cstrOri = m_cstrOriUrl ;
			CString cstrInput = pDispParams->rgvarg[0].pvarVal->bstrVal ;
			int i = cstrOri.Find(L"://") ;
			if (i > 0)
				cstrOri = cstrOri.Right(cstrOri.GetLength() - 3 - i) ;

			m_cstrConnectedUrl = m_cstrOriUrl ;

			BOOL bSamilarUrl = FALSE ;
			CString cstrLastUrl = m_cstrLastUrl ;
			CString cstrCurUrl = m_cstrConnectedUrl ;
			do 
			{
				i = cstrLastUrl.Find('#') ;
				if (i > 0)
					cstrLastUrl = cstrLastUrl.Left(i) ;
				i = cstrCurUrl.Find('#') ;
				if (i > 0)
					cstrCurUrl = cstrCurUrl.Left(i) ;
				if (0 == cstrCurUrl.Compare(cstrLastUrl))
				{
					bSamilarUrl = TRUE ;
					break ;
				}

				i = cstrLastUrl.Find('?') ;
				if (i > 0)
					cstrLastUrl = cstrLastUrl.Left(i) ;
				i = cstrCurUrl.Find('?') ;
				if (i > 0)
					cstrCurUrl = cstrCurUrl.Left(i) ;
				if (0 == cstrCurUrl.Compare(cstrLastUrl))
				{
					bSamilarUrl = TRUE ;
					break ;
				}
			}
			while (0);
		}

		if (!wcsncmp(L"http:", pDispParams->rgvarg[0].pvarVal->bstrVal, 5)   ||
			!wcsncmp(L"https:", pDispParams->rgvarg[0].pvarVal->bstrVal, 6)  )
		{
			// ������ҳ����������,//ֻ�е�ҳ��Ϊhttp��https�Ĳż���ڲ��ڰ�������
			CString strUrl(pDispParams->rgvarg[0].pvarVal->bstrVal);
			if (NULL == CURLList::GetInstance()->GetData(L"", strUrl))
			{
				CListManager::NavigateWebPage(strUrl);
				::PostMessage(m_pAxControl->m_hChildFrame, WM_ITEM_AUTOCLOSE, 0, 0);
				return S_OK;
			}
		}		


		m_cstrLastUrl = pDispParams->rgvarg[0].pvarVal->bstrVal ;

		::SendMessage(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_SET_TAB_URL, (LPARAM)pDispParams->rgvarg[0].pvarVal->bstrVal);
		m_pDispDocument = pDispParams->rgvarg[1].pdispVal;
	}

	return S_OK ;
}

HRESULT CWebBrowserEventsManager::OnDocumentComplete(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	//�������������ײ㣬��Ϊֻ�е��������¼�ʱ�����ܷ��ý��㣬��������Ĵ��ڻ�û����
	LONG style = ::GetWindowLongW(m_pAxControl->m_hWnd, GWL_STYLE);
	if((style & WS_VISIBLE) ==  WS_VISIBLE)
	{
		HWND TChild, MChild, BChild;
		TChild = FindWindowExW(m_pAxControl->m_hWnd , NULL, L"Shell Embedding", NULL);
		if(TChild != NULL)
		{
			MChild = FindWindowExW(TChild, NULL, L"Shell DocObject View", NULL);
			if(MChild != NULL)
			{
				BChild = FindWindowExW(MChild, NULL, L"Internet Explorer_Server", NULL);
				if(BChild != NULL)
				{
					::SetFocus(BChild);
				}
			}
		}
	}

	if (pDispParams->cArgs < 2)
	{
		return E_FAIL;
	}

	int state = 0;
	if(m_isGetBill)
	{
		LPURLLIST pUrlList = CBillUrlManager::GetInstance()->GetUrlMap(m_pBillData->aid, m_pBillData->type);
		map<int, wstring>* purlmap =  &(pUrlList->url);
		//////////////////////////////////////////////////////////////////////////
		IWebBrowser2 *thisBrowser	= NULL;
		IDispatch *docDisp			= NULL;
		HRESULT hr = S_OK;

		IDispatch* pBrowser = pDispParams->rgvarg[1].pdispVal;
		hr = pBrowser->QueryInterface(IID_IWebBrowser2, reinterpret_cast<void **>(&thisBrowser));

		int maxnum = 0; 
		if( CBillUrlManager::GetInstance()->Getmode(m_pBillData->aid) )
		{
			CString   strtmpqq;
       		CString   strtmp = (CString)pDispParams->rgvarg[0].pvarVal->bstrVal;
			map<int, wstring>::iterator ite;
			for(ite = purlmap->begin(); ite != purlmap->end(); ite ++)
			{
				if(ite->first > 2)
				{
					wstring url = ite->second;   
					strtmpqq = strtmp.Mid(0,url.size());
					if( find(pUrlList->m_beginstep.begin(), pUrlList->m_beginstep.end(), ite->first) != pUrlList->m_beginstep.end())//
						strtmpqq = strtmp;
					if(strtmpqq == url.c_str())
					{
						m_step = ite->first;
    					maxnum = BILL_BROWSER_GO;
					}
				}
			}

	        if (SUCCEEDED(hr) && thisBrowser != NULL) 
			{	
				// �ж��Ƿ��ǵ�ǰҳ�������ɣ�������״̬��browser�������õ���ɼ��ص�����
				if(/*m_pWebBrowser2 == thisBrowser ||*/ maxnum == BILL_BROWSER_GO )
				{
					READYSTATE rstate;
					hr = thisBrowser->get_ReadyState(&rstate);
					if(hr == S_OK && rstate == READYSTATE_COMPLETE)
					{
						//��ʼ�жϸ��߳��Ƿ��Ǽ��˹��ܵ�����߳�
						if(m_pBillData)
						{
							READYSTATE rstate;
							hr = thisBrowser->get_ReadyState(&rstate);
							if(hr == S_OK && rstate == READYSTATE_COMPLETE)
							{
								//��ʼ�жϸ��߳��Ƿ��Ǽ��˹��ܵ�����߳�
								if(m_pBillData)
								{
									//if(m_pWebBrowser2 == thisBrowser && m_step < BILL_LOGIN_SUCC && CBillUrlManager::GetInstance()->Getislogin(m_pBillData->aid) == BILL_LOGIN_SUCC)
									//	m_step = BILL_LOGIN_SUCC;
									CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_GET_BILL, CRecordProgram::GetInstance()->GetRecordInfo(L"�����˵�����%d", m_step));
									m_bCanClose = false;
									state = CBillUrlManager::GetInstance()->GetBill(m_pWebBrowser2,thisBrowser, m_pBillData, m_step, m_pAxControl->m_hWnd);
									m_bCanClose = true;

									bool needRestart = false;
									ShowUserGetBillState(state, needRestart);
									if(needRestart == true )
									{
										m_bCanClose = false;
										state = CBillUrlManager::GetInstance()->GetBill(m_pWebBrowser2, thisBrowser, m_pBillData, m_step, m_pAxControl->m_hWnd);
										m_bCanClose = true;
									}
								}
							}
						}
					}
					maxnum = 0;
				}
       		}	

		}
		else if (SUCCEEDED(hr) && thisBrowser != NULL) 
		{	
			// �ж��Ƿ��ǵ�ǰҳ�������ɣ�������״̬��browser�������õ���ɼ��ص�����
			if(m_pWebBrowser2 == thisBrowser|| m_step > 1)
			{
				READYSTATE rstate;
				hr = thisBrowser->get_ReadyState(&rstate);
				if(hr == S_OK && rstate == READYSTATE_COMPLETE)
				{
					//��ʼ�жϸ��߳��Ƿ��Ǽ��˹��ܵ�����߳�
					if(m_pBillData)
					{
						bool needRestart = false;
						while( true )
						{
							m_bCanClose = false;
							state = CBillUrlManager::GetInstance()->GetBill(m_pWebBrowser2, thisBrowser,  m_pBillData, m_step, m_pAxControl->m_hWnd);
							m_bCanClose = true;
							ShowUserGetBillState(state, needRestart);

							if(false == needRestart)
								break;
							else
							{
								m_step ++;
							}
						}
					}
				}
			}
		}	
		if (thisBrowser) { thisBrowser->Release(); thisBrowser = NULL; }
	}


/* ��ʱ�رոò��ִ��룬�Ժ���ӵ�����ͺ���js�����ҳ����
	//////////////////////////////////////////////////////////////////////////
	// ���� onerror �¼�
	IWebBrowser2 *thisBrowser	= NULL;
	IDispatch *docDisp			= NULL;
	IHTMLDocument3 *doc			= NULL;
	IHTMLElement *elem			= NULL;
	HRESULT hr = S_OK;

	IDispatch* pBrowser = pDispParams->rgvarg[1].pdispVal;
	hr = pBrowser->QueryInterface(IID_IWebBrowser2, reinterpret_cast<void **>(&thisBrowser));
	if (FAILED(hr) || thisBrowser == NULL) goto cleanup;
	hr = thisBrowser->get_Document(&docDisp);
	if (FAILED(hr) || docDisp == NULL) goto cleanup;
	hr = docDisp->QueryInterface(IID_IHTMLDocument3, reinterpret_cast<void**>(&doc));
	if (FAILED(hr) || doc == NULL) goto cleanup;
	if (docDisp) { docDisp->Release(); docDisp = NULL; }

	hr = doc->get_documentElement(&elem);
	if (FAILED(hr) || elem == NULL) goto cleanup;
	hr = elem->insertAdjacentHTML(L"afterBegin", L"&#xa0;<script for='window' event='onerror'>var noOp = null;</script>");
	// Cleanup.
cleanup:
	if (elem) { elem->Release(); elem = NULL; }
	if (thisBrowser) { thisBrowser->Release(); thisBrowser = NULL; }
	if (doc) { doc->Release(); doc = NULL; }
*/
	//////////////////////////////////////////////////////////////////////////
	//
	
	if(state == 0)
	{
		if (IsToppestWindowOfBrowser2(pDispParams->rgvarg[1].pdispVal))
		{
			if(FileterNormalUrl( pDispParams->rgvarg[0].pvarVal->bstrVal))
				return S_OK ;
			::SendMessage(m_pAxControl->m_hChildFrame, WM_SETTEXT, WM_ITEM_SET_TAB_URL, (LPARAM)pDispParams->rgvarg[0].pvarVal->bstrVal);
		}
	}
	if(BILL_CANCEL_GET_BILL == m_iCancelState || BILL_EXCEED_MAX_TIME == m_iCancelState)//ȡ���ͳ�ʱ�Ļ���ô��
		state = m_iCancelState;

	if(state >= BILL_COM_ERROR && state <= BILL_COM_ERROR + 100)//����
	{
		HWND hMainFrame = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
		//��Ҫ��
		CBillUrlManager::GetInstance()->FreeDll();
		CloseNotifyWnd();
		if(BILL_COM_ERROR == state)
			MessageBox(hMainFrame, L"�����˵�ʧ�ܣ���������������״�����ѣ���������ϵͳ����\r\n�����Գ������µ�����������ƽ��", L"�˵�����", MB_OK);
		else if(BILL_GET_ACCOUNT_ERROR == state)
			MessageBox(hMainFrame, L"��ȡ�˺�ʧ�ܣ���������������״�����ѣ���������ϵͳ����\r\n�ƽ���޷������˵�����������ܣ������Գ������µ�����������ƽ��", L"�˵�����", MB_OK);
		FinishGettingBill();
	}
	else if(state == BILL_ALL_FINISH || state == BILL_CANCEL_GET_BILL)//�����رպ�ȡ������Ĵ���ʽ��ͬ
	{
		CBillUrlManager::GetInstance()->FreeDll();
		FinishGettingBill();
	}

	else if(state == BILL_EXCEED_MAX_TIME)//��ʱ����
	{
		HWND hMainFrame = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
		CBillUrlManager::GetInstance()->FreeDll();
		
		MessageBox(hMainFrame, L"�˵�����ʧ�ܣ������½����˵�����", L"�ƽ��", MB_OK);
		
		FinishGettingBill();//�ر�ҳ��
	}

	else if(state == BILL_INNER_CANCEL)
	{
		CBillUrlManager::GetInstance()->FreeDll();
		CloseNotifyWnd();
		FinishGettingBill();
	}

	return S_OK ;
}
void CWebBrowserEventsManager::SetNotifyWnd(HWND nHwnd)
{
	m_notifyWnd = nHwnd;
	CBillUrlManager::GetInstance()->SetNotifyWnd(nHwnd);
}
void CWebBrowserEventsManager::CancelGetBill()
{
	m_iCancelState = BILL_CANCEL_GET_BILL;
	CBillUrlManager::GetInstance()->SetGetBillState(bSCancel);
	if(m_bCanClose)
	{
		CBillUrlManager::GetInstance()->FreeDll();
		FinishGettingBill();
	}

}
void CWebBrowserEventsManager::GetBillExceedTime()
{
	CBillUrlManager::GetInstance()->SetGetBillState(bSExceedTime);
}
//������4������Ϊ������ʾ�ĺ���
void CWebBrowserEventsManager::CloseNotifyWnd() //�ر����ڵ�½�����˵��Ի���
{
	m_notifyWnd = 0;
	::SendMessage(m_pAxControl->m_hWnd, WM_AX_END_INFO_DLG, 0, 0);
}
void CWebBrowserEventsManager::ShowNotifyWnd(InfoDlgType type)//�������ڵ�½�����˵��Ի���
{
	if(type == dLoginDlg)
		::SendMessage(m_pAxControl->m_hWnd, WM_AX_SHOW_INFO_DLG, 0, 0);
	else if(type == dGettingDlg)
	{
		::SendMessage(m_pAxControl->m_hWnd, WM_AX_SHOW_INFO_DLG, 1, 0);
	}
	int i = 0;
	while((i < 10) && (m_notifyWnd == 0))
	{
		i ++;
		Sleep(200);
	}
}
void CWebBrowserEventsManager::NotifyGettingBill() //��ʼ�����˵�
{
	::SendMessage(m_pAxControl->m_hChildFrame, WM_GETTING_BILL, BILL_LOAD_DLG, 0);
}
void CWebBrowserEventsManager::FinishGettingBill()//��ɵ����˵����رո�����Tab
{
	::SendMessage(m_pAxControl->m_hChildFrame, WM_FINISH_GET_BILL, BILL_FINISH_STATE, 0);//�ر�ҳ��
}
void CWebBrowserEventsManager::RebeginGetBill()//��ɵ����˵����رո�����Tab
{
	::SendMessage(m_pAxControl->m_hChildFrame, WM_RE_GETBILL, BILL_FINISH_STATE, 0);//�ر�ҳ��
}

int CWebBrowserEventsManager::ShowUserGetBillState( int& state, bool& needRestart)
{
	needRestart = false;
	int nBack = 0;
	bool bNoBreak = false;
	switch(state)
	{
	case BLII_NEED_RESTART:
		{
			CloseNotifyWnd();//�رմ���
	     	CBillUrlManager::GetInstance()->InitDll();
			m_bCanClose = true;
			
			m_stepTime.clear();
			m_iCancelState = 0;
			m_step = 0;
			CComVariant var;
			bool isBeginStep;
			wstring url = CBillUrlManager::GetInstance()->GetBillUrl(m_pBillData->aid, m_pBillData->type, 1, isBeginStep);
			m_pWebBrowser2->Navigate((BSTR)url.c_str(), &var, &var, &var, &var);
			RebeginGetBill();
		break;
		}

	case BILL_SELECT_ACCOUNT_MONTH2: // ѡ���˻����·�һ�鵯��
			bNoBreak = true;
	case BILL_SELECT_ACCOUNT:// ��ʾѡ���˻�����
		{
			CloseNotifyWnd();//�رմ���

			// �����˻�ѡ�����
			if(::SendMessage(m_pAxControl->m_hWnd, WM_AX_ACCOUNT_SELECT, 1, (LPARAM)&(CBillUrlManager::GetInstance()->GetBillRecords()->m_mapBack)) != IDCANCEL) // ��list��ָ��ͨ��lParam����, wParam = 1��ʾѡ���˻�����
			{
				//state = BILL_CANCEL_GET_BILL; // ȡ���˵�����
				needRestart = true;
			}
			else
			{
				state = BILL_CANCEL_GET_BILL; // ȡ���˵�����
				needRestart = false;
			}
			
		}
		if (!bNoBreak)
			break;
	case BILL_SELECT_MONTH2:
		{
			CloseNotifyWnd();//�ر���ʾ����
			USES_CONVERSION;
			wstring begintime = A2W(m_pBillData->select);

			CSelectMonthDlg dlg;
			HWND hMainFrame = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
			dlg.SetBeginTime(begintime);
			if(dlg.DoModal(hMainFrame) == IDOK)
			{
				string btime,etime;
				dlg.GetSelectTime(btime, etime);
				btime += etime;
				memset(m_pBillData->select, 0, sizeof(m_pBillData->select));
				strcpy_s(m_pBillData->select, 256, btime.c_str());
				needRestart = true;
				// �½��������ڵ����˵�����
				ShowNotifyWnd(dGettingDlg);
			}
			else
			{
				state = BILL_CANCEL_GET_BILL; // ȡ���˵�����
				needRestart = false;				
			}

			Sleep(20);
		
			break;
		}
	case BILL_DOWNLOAD_DLG:// ��ʾ���ڵ����˵�����
		::PostMessage(m_pAxControl->m_hChildFrame,WM_GETTING_BILL, 0, 0);
		break;
	case BILL_SELECT_MONTH:// ��ʾ�·�ѡ�����
		{
			CloseNotifyWnd();//�رմ���

			list<SELECTINFONODE>::iterator cstIt = CBillUrlManager::GetInstance()->GetBillRecords()->m_mapBack.begin();
			for (; cstIt != CBillUrlManager::GetInstance()->GetBillRecords()->m_mapBack.end(); cstIt ++)
			{
				// ����ѵ�����
				if (CBankData::GetInstance()->IsMonthImport(CBillUrlManager::GetInstance()->GetBillRecords()->tag, cstIt->szNodeInfo, m_pBillData->accountid))
					(*cstIt).dwVal |= CHECKBOX_SHOW_CHECKED_BEFORE;
				else
					(*cstIt).dwVal = CHECKBOX_SHOW_CHECKED;//Ĭ��ȫѡ
					
			}

			//�����·�ѡ�����
			if(::SendMessage(m_pAxControl->m_hWnd, WM_AX_ACCOUNT_SELECT, 0, (LPARAM)&(CBillUrlManager::GetInstance()->GetBillRecords()->m_mapBack)) != IDCANCEL)
			{				
				needRestart = true;				
				ShowNotifyWnd(dGettingDlg);
			}
			else
			{
				state = BILL_CANCEL_GET_BILL; // �����˵�����
			}
			break;
		}
	case BILL_BROWSER_LOOP:
		{
			needRestart = true;
		}
		break;

	default:
		break;
	}

	return nBack;
}



HRESULT CWebBrowserEventsManager::OnProgressChange(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (pDispParams->cArgs < 2)
	{
		return E_FAIL;
	}
	if (pDispParams->rgvarg[0].lVal == 0 && pDispParams->rgvarg[1].lVal == 0)
	{
		m_dwPercentage = 0 ;
		return S_OK ;
	}

	if(m_isGetBill)
		if(m_step > 1)
			return S_OK;

	long lMax = pDispParams->rgvarg[0].lVal ;
	long lNow = pDispParams->rgvarg[1].lVal ;

	if (0 == lMax || 0 == lNow)
	{
		::PostMessage(m_pAxControl->m_hChildFrame, WM_ITEM_SET_PAGE_PROGRESS, 100, 0);

		return S_OK ;
	}

	if (-1 == lNow)
	{
		m_dwPercentage = 100;
	}
	else
	{
		m_dwPercentage = (DWORD)((lNow * 100)/lMax) > m_dwPercentage ? (DWORD)((lNow * 100)/lMax) : m_dwPercentage ;
		m_dwPercentage = m_dwPercentage > 100 ? 100 : m_dwPercentage ;

		//m_pAxControl->SetTimer(0x4, 2000);
	}

	::PostMessage(m_pAxControl->m_hChildFrame, WM_ITEM_SET_PAGE_PROGRESS, m_dwPercentage, 0);

	return S_OK ;
}

HRESULT CWebBrowserEventsManager::OnQuit(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	return S_OK;
}

HRESULT CWebBrowserEventsManager::OnNavigateError(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if(FileterNormalUrl( pDispParams->rgvarg[3].pvarVal->bstrVal))
		return S_OK ;

	wstring  strFileter = pDispParams->rgvarg[3].pvarVal->bstrVal;
	if(CHostContainer::GetInstance()->IsUrlInUrlError( strFileter ))
		return S_OK ;

	IDispatch* pDisp = pDispParams->rgvarg[4].pdispVal ;
	if (pDisp == NULL)
		return S_OK;
	if(pDispParams->rgvarg[2].pvarVal->vt == VT_BSTR)
	{
		if(pDispParams->rgvarg[2].pvarVal->bstrVal != NULL && wcslen(pDispParams->rgvarg[2].pvarVal->bstrVal) >= 9)
		{
			if((!wcsncmp(L"newwindow", pDispParams->rgvarg[2].pvarVal->bstrVal, 9)) || (!wcsncmp(L"loginFrame", pDispParams->rgvarg[2].pvarVal->bstrVal, 10)) ||
				(!wcsncmp(L"LoginFrame", pDispParams->rgvarg[2].pvarVal->bstrVal, 10)))// ���ڽ�����ҳ��Ĳ���������
				return S_OK;
		}
	}
	
	// �ڼ��˹����г��ִ�����ô֪ͨ�û�������Ҫ�к�������
	// �رյ�ǰҳ���Ǵ���ҳ����Ҫ����
	if(m_isGetBill)
		if(m_step > 1)
			return S_OK;

	if((int)pDispParams->rgvarg[1].pvarVal->uiVal == 403)
		return S_OK;

	// [TuotuoXP] ���Ĵ���ҳ��
	CComPtr<IWebBrowser2> spWebBrowser2;
	HRESULT hr = pDisp->QueryInterface(IID_IWebBrowser2, (void**)&spWebBrowser2);
	if (SUCCEEDED(hr) && spWebBrowser2)
	{
		CComPtr<IDispatch> spDispDoc;
		hr = spWebBrowser2->get_Document(&spDispDoc);
		if (SUCCEEDED(hr) && spDispDoc)
		{
			CComPtr<IHTMLDocument2> spHTMLDocument2;
			hr = spDispDoc->QueryInterface(IID_IHTMLDocument2, (void**)&spHTMLDocument2);
			if (SUCCEEDED(hr) && spHTMLDocument2)
			{
				TCHAR szPath[MAX_PATH];
				_stprintf_s(szPath, _T("%s\\Html\\Error\\ErrorPage.html"), ::GetModulePath());
				std::fstream fs;
				fs.open(szPath, std::ios::in);
				if (fs.good())
				{
					std::stringstream ss;
					ss << fs.rdbuf();
					std::string strContent = ss.str();

					size_t n = strContent.find("$ERRCODE2$");
					if (n != std::string::npos)
					{
						std::stringstream ssnum;
						ssnum << (int)pDispParams->rgvarg[1].pvarVal->uiVal;
						CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_STATE, CRecordProgram::GetInstance()->GetRecordInfo(L"NavigateError:%s:%d",strFileter.c_str(),(int)pDispParams->rgvarg[1].pvarVal->uiVal));
						strContent = strContent.replace(n, 10, ssnum.str());
					}

					n = strContent.find("error.png");
					if (n != std::string::npos)
					{
						_stprintf_s(szPath, _T("file:///%s\\Html\\Error\\error.png"), ::GetModulePath());
						strContent = strContent.replace(n, 9, WToA(szPath));
					}

					SAFEARRAY *psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
					VARIANT *param;
					HRESULT hr = SafeArrayAccessData(psaStrings, (LPVOID*)&param);
					param->vt = VT_BSTR;
					param->bstrVal = ::SysAllocString(AToW(strContent.c_str()).c_str());

					hr = SafeArrayUnaccessData(psaStrings);
					hr = spHTMLDocument2->write(psaStrings);
					if (psaStrings != NULL) 
						SafeArrayDestroy(psaStrings);

					*pDispParams->rgvarg[0].pboolVal = VARIANT_TRUE;
					m_error = true;
				}
			}
		}
	}

	return S_OK ;
}
std::string CWebBrowserEventsManager::GetFilterFile()
{
	TCHAR szPath[MAX_PATH] = { 0 };

	::GetModuleFileName(NULL, szPath, _countof(szPath));
	PathRemoveFileSpecW(szPath);

	std::wstring wcsPath(szPath);
	wcsPath += L"\\syslog.txt";
	std::string info;
	//DWORD dwReadNum;
	HANDLE hFile;

	if( hFile = CreateFileW(wcsPath.c_str(),GENERIC_READ,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_READONLY,NULL))
	{
		//if(hFile == INVALID_HANDLE_VALUE)
		//	return info;

		//const int bufsize = 2 * 1024 * 1024;
		//wchar_t* buf = new wchar_t[bufsize];
		//if(buf == NULL)
		//	return info;
		//DWORD fsize = bufsize * sizeof(wchar_t);

		//if(ReadFile(hFile,buf,fsize,&dwReadNum,NULL))
		//{
		//	wchar_t *p = buf;
		//	wstring info(p);			
		//	p = wcsstr(buf,L"Fileter Module :");//Ѱ�ҵ�һ��
		//	while(p)
		//	{
		//		wchar_t *q = wcsstr(p,L"\r\n");

		//		wchar_t allstr[255] = {0};
		//		memcpy(allstr,p,(q - p)*2);
		//		std::wstring tmp(allstr);

		//		info += "  " + ws2s(tmp);
		//		p = wcsstr(q,L"Fileter Module :");
		//	}
		//}
		//delete[] buf;

		::CloseHandle(hFile);
	}

	return info;
}

std::string CWebBrowserEventsManager::ws2s(const std::wstring& ws)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
	if (nLen<= 0) return std::string("");
	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");
	WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen -1] = 0;
	std::string strTemp(pszDst);
	delete [] pszDst;
	return strTemp;
}

HRESULT CWebBrowserEventsManager::OnSetSecureLockIcon(REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (pDispParams->cArgs < 1)
		return E_FAIL ;

	if(m_isGetBill)
		if(m_step > 1)
			return S_OK;

	BOOL bSecure = (pDispParams->rgvarg[0].lVal >= secureLockIconMixed) ? TRUE : FALSE;
	::PostMessage(m_pAxControl->m_hChildFrame, WM_ITEM_SET_SSL_STATE, 0, (LPARAM)bSecure);

	return S_OK;
}

BOOL CWebBrowserEventsManager::IsToppestWindowOfBrowser2(IDispatch* pDispatch)
{
	if (!pDispatch || !m_pWebBrowser2)
	{
		return FALSE ;
	}

	CComPtr<IUnknown> pUnk;
	CComPtr<IDispatch> pWBDisp;

	if (FAILED(m_pWebBrowser2->QueryInterface(IID_IUnknown, (void**)&pUnk)) || !pUnk)
	{
		return FALSE ;
	}

	if (FAILED(pUnk->QueryInterface(IID_IDispatch, (void**)&pWBDisp)) || !pWBDisp)
	{
		return FALSE ;
	}

	if (pDispatch == pWBDisp)
	{
		return TRUE ;
	}

	return FALSE ;
}

BOOL CWebBrowserEventsManager::IsToppestWindowOfBrowser(IDispatch* pDispatch)
{
	if (!pDispatch)
	{
		return FALSE ;
	}

	HRESULT hr = S_OK ;
	CComPtr<IWebBrowser2> spWebBrowser2 ;
	hr = pDispatch->QueryInterface(IID_IWebBrowser2, (void**)&spWebBrowser2) ;
	if (FAILED(hr) || !spWebBrowser2)
	{
		return FALSE ;
	}
	CComPtr<IDispatch> spDispDoc ;
	hr = spWebBrowser2->get_Document(&spDispDoc) ;
	if (FAILED(hr) || !spDispDoc)
	{
		return FALSE ;
	}
	CComPtr<IHTMLDocument2> spHTMLDocument2 ;
	hr = spDispDoc->QueryInterface(IID_IHTMLDocument2, (void**)&spHTMLDocument2) ;
	if (FAILED(hr) || !spHTMLDocument2)
	{
		return FALSE ;
	}
	CComPtr<IHTMLWindow2> spHTMLWindow2 ;
	hr = spHTMLDocument2->get_parentWindow(&spHTMLWindow2) ;
	if (FAILED(hr) || !spHTMLWindow2)
	{
		return FALSE ;
	}

	CComPtr<IHTMLWindow2> spTopWindow2 ;
	hr = spHTMLWindow2->get_top(&spTopWindow2) ;

	if (FAILED(hr) || !spHTMLWindow2)
	{
		return FALSE ;
	}
	//////////////////////////////////////////////////////////////////////////
	CComPtr<IObjectIdentity> spIdentity ;
	hr = spHTMLWindow2->QueryInterface(IID_IObjectIdentity, (void**)&spIdentity) ;
	if (FAILED(hr) || !spHTMLWindow2)
	{
		return FALSE ;
	}
	hr = spIdentity->IsEqualObject(spTopWindow2) ;
	if (S_OK == hr)
	{
		m_dwHTMLWindowBaseAddr = *(DWORD*)spHTMLWindow2.p ;
		return TRUE ;
	}

	return FALSE ;
}

BOOL CWebBrowserEventsManager::IsValidHttpUrl(const BSTR bstrUrl)
{
	CString strUrl(bstrUrl);
	strUrl.MakeLower();
	if (strUrl.Left(5) == _T("http:") || strUrl.Left(6) == _T("https:"))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CWebBrowserEventsManager::FileterNormalUrl(BSTR bstrUrl)
{
	if (!wcsncmp(L"javascript:", bstrUrl, 11)   ||
		!wcsncmp(L"vbscript:", bstrUrl, 9)      ||
		!wcsncmp(L"mailto:", bstrUrl, 7)        ||
		!wcsncmp(L"about:", bstrUrl, 6)          )
	{
		return TRUE ;
	}
	return FALSE ;
}