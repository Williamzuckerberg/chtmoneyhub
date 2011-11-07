#include "stdafx.h"
#include "ConvertBase.h"
#include "AxControl.h"
#include "ExternalDispatchImpl.h"
#include "..\BankData\BankData.h"
#include "..\Utils\ListManager\ListManager.h"
#include "..\Utils\UserBehavior\UserBehavior.h"
#include "..\Utils\Config\HostConfig.h"
#include "..\BankUI\Util\ProcessManager.h"
#include "..\BankUI\UIControl\SelectMonthDlg.h"
#include "..\BankUI\UIControl\CoolMessageBox.h"
#include "BankData\BkInfoDownload.h"
#include "GetBill\BillUrlManager.h"

#define CMD_DELETE_FAVORITE		L"DeleteFav"
#define CMD_SAVE_FAVORITE		L"SaveFav"
#define CMD_GET_FAVORITE		L"GetFav" //��ʼ����ȡFav�Ľӿ�
#define CMD_CHANGEORDER_FAV		L"ChangeOrder" // ���ղص����н����϶�

#define CMD_ADD_EVENT			L"AddEvent"
#define CMD_DELETE_EVENT		L"DeleteEvent"
#define CMD_GET_EVENTS			L"GetEvents"
#define CMD_GET_EVENTS_ONEDAY	L"GetEventsOneDay"

#define CMD_SET_ALARM			L"SetAlarm"
#define CMD_GET_TODAYALARMS		L"GetTodayAlarms"

#define CMD_SAVE_COUPON			L"SaveCoupon"
#define CMD_DELETE_COUPON		L"DeleteCoupon"
#define CMD_GET_COUPONS			L"GetCoupons"

#define CMD_GET_APPDATA			L"GetAppData"
#define CMD_PRINT_HTML			L"PrintHtml"

#define CMD_SEND_VISITRECORD	L"SendVisitRecord"
#define CMD_GET_ENVIRONMENT		L"GetEnvironment"
#define CMD_REFRESH_PAGE		L"RefreshPage"
#define CMD_HOST_NAME			L"GetHostName"


#define CMD_QUERY_SQL			L"QuerySQL"
#define CMD_EXECUTE_SQL			L"ExecuteSQL"
#define CMD_GET_XML_DATA		L"GetXMLData"
#define CMD_GET_PHP_DATA		L"GetPhpData"

#define CMD_GET_BILL_LIST		L"sendGetBillInterface"
#define CMD_WRITE_LOG			L"WriteLog"

#define CMD_SET_JS_PARAM		L"SetParameter" // �ž������ӿ�2011-09-09
#define CMD_GET_JS_PARAM		L"GetParameter"
#define CMD_GET_SCREEN_SIZE		L"GetScreenSize"
//#define CMD_GET_SCREEN_HEIGHT	L"GetScreenHeight"

#define CMD_SHOWWAITWINDOW		L"ShowWaitWindow"

#define DISPID_SAVE_FAVORITE	12346
#define DISPID_DELETE_FAVORITE	12347
#define DISPID_GET_FAVORITE		12350
#define DISPID_CHANGEORDER_FAV	12351

#define DISPID_ADD_EVENT		12360
#define DISPID_DELETE_EVENT		12361
#define DISPID_GET_EVENTS		12362
#define DISPID_GET_EVENTSONEDAY	12363

#define DISPID_SET_ALARM		12371
#define DISPID_GET_TODAYALARMS	12372

#define DISPID_SAVE_COUPON		12381
#define DISPID_DELETE_COUPON	12382
#define DISPID_GET_COUPONS		12383

#define DISPID_GET_APPDATA		12400
#define DISPID_PRINT_HTML		12401

#define DISPID_SEND_VISITRECORD 12410
#define DISPID_GET_ENVIRONMENT	12411
#define DISPID_REFRESH_PAGE		12412
#define DISPID_HOST_NAME		12413

#define DISPID_QUERY_SQL		12420
#define DISPID_EXECUTE_SQL		12421
#define DISPID_GET_XML_DATA		12422
#define DISPID_GET_PHP_DATA		12423
#define DISPID_SET_JS_PARAM		12424 // �ž������ӿ�2011-09-09
#define DISPID_GET_JS_PARAM		12425
#define DISPID_GET_SIZE			12426
//#define DISPID_GET_HEIGHT		12427

#define DISPID_GET_BILL_LIST	12500

#define DISPID_WRITE_LOG		12600
#define DISPID_SHOWWAITWINDOW	12610


extern HWND g_hMainFrame;

HWND CExternalDispatchImpl::m_hFrame[3] = {NULL, NULL, NULL};
HWND CExternalDispatchImpl::m_hAxui = 0;
HANDLE CExternalDispatchImpl::m_logHandle = NULL;
CRect CExternalDispatchImpl::s_rectClient = CRect(0, 0, 0, 0);
std::list<std::string> CExternalDispatchImpl::m_sstrVerctor;
std::map<std::string, std::string> CExternalDispatchImpl::m_mapParam;
CExternalDispatchImpl::CExternalDispatchImpl(CAxControl *pAxControl) : m_pAxControl(pAxControl)
{
}

CExternalDispatchImpl::~CExternalDispatchImpl()
{
}

STDMETHODIMP_(ULONG) CExternalDispatchImpl::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG) CExternalDispatchImpl::Release()
{
	return 1;
}

STDMETHODIMP CExternalDispatchImpl::QueryInterface(const IID &riid, void **ppvObject)
{
	if (riid == IID_IUnknown) 
	{ 
		AddRef() ;
		*ppvObject = (void*)(this); 
		return S_OK; 
	} 
	else if (riid == IID_IDispatch)
	{
		AddRef() ;
		*ppvObject = (IDispatch*)(this); 
		return S_OK; 
	}

	return E_NOINTERFACE ;
}

// IDispatch
STDMETHODIMP CExternalDispatchImpl::GetTypeInfoCount(UINT *pctinfo)
{
	return E_NOTIMPL ;
}

STDMETHODIMP CExternalDispatchImpl::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	return E_NOTIMPL ;
}

STDMETHODIMP CExternalDispatchImpl::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
	HRESULT hr = NOERROR;

	for (UINT i = 0; i < cNames; i++)
	{
		if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SAVE_FAVORITE, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SAVE_FAVORITE;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_DELETE_FAVORITE, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_DELETE_FAVORITE;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_FAVORITE, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_FAVORITE;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_CHANGEORDER_FAV, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_CHANGEORDER_FAV;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_ADD_EVENT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_ADD_EVENT;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_DELETE_EVENT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_DELETE_EVENT;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_EVENTS, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_EVENTS;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_EVENTS_ONEDAY, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_EVENTSONEDAY;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SET_ALARM, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SET_ALARM;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_TODAYALARMS, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_TODAYALARMS;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SAVE_COUPON, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SAVE_COUPON;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_DELETE_COUPON, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_DELETE_COUPON;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_COUPONS, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_COUPONS;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_APPDATA, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_APPDATA;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_PRINT_HTML, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_PRINT_HTML;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SEND_VISITRECORD, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SEND_VISITRECORD;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_ENVIRONMENT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_ENVIRONMENT;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_REFRESH_PAGE, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_REFRESH_PAGE;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_HOST_NAME, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_HOST_NAME;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_QUERY_SQL, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_QUERY_SQL;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_EXECUTE_SQL, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_EXECUTE_SQL;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_XML_DATA, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_XML_DATA;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_PHP_DATA, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_PHP_DATA;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SET_JS_PARAM, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SET_JS_PARAM;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_JS_PARAM, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_JS_PARAM;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_SCREEN_SIZE, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_SIZE;
		}
		/*else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_SCREEN_HEIGHT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_HEIGHT;
		}*/
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_BILL_LIST, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_BILL_LIST;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_WRITE_LOG, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_WRITE_LOG;
		}
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SHOWWAITWINDOW, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SHOWWAITWINDOW;
		}
		else
		{
			hr = ResultFromScode(DISP_E_UNKNOWNNAME);
			rgDispId[i] = DISPID_UNKNOWN;
		}
	}

	return hr;
}

DWORD WINAPI CExternalDispatchImpl::DownloadBkCtrlThreadProc(LPVOID lpParam)
{
	if (NULL == lpParam)
		return 0;

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"������DownloadBkCtrlThreadProc");
	CBkInfoDownloadManager *pTemp = CBkInfoDownloadManager::GetInstance ();
	std::string strTempID = (char*)lpParam;
	AddUserFavBank(strTempID); // ��¼�û��ղ����е�ID
	pTemp->MyBankCtrlDownload ((char*)lpParam);
	RemoveUserFavBank(strTempID); // �Ƴ��û��ղص�ID
	char *lp = (char*)lpParam;
	delete[] lp;
	return 0;
}

STDMETHODIMP CExternalDispatchImpl::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	USES_CONVERSION;

	if (dispIdMember == DISPID_SAVE_FAVORITE && (wFlags & DISPATCH_METHOD))
	{
		CBankData* pBankData = CBankData::GetInstance();

		//std::string strFav;
		wstring wInfo = pDispParams->rgvarg[0].bstrVal;
		m_strBkID = WToA(wInfo, 936);
		
		//m_strBkID = OLE2A(pDispParams->rgvarg[0].bstrVal);
		if (m_strBkID.find ("&") > 0)
			m_strBkID = m_strBkID.substr (0, m_strBkID.find ("&"));

		pBankData->SaveFav(m_strBkID, 0);// �����ղ�, 0��ʾû������

		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"���һ���ղ�");
		CRecordProgram::GetInstance()->RecordDebugInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, A2W(m_strBkID.c_str()));

		// ִ�����пؼ�����
	/*
		CBkInfoDownloadManager *pTemp = CBkInfoDownloadManager::GetInstance ();
		pTemp->MyBankCtrlDownload ((char*)strFav.c_str ());*/
		
		//
		DWORD dwThreadID = 0;
		char* appid = new char[20];
		memset(appid, 0, 20);
		strcpy_s(appid, 20, m_strBkID.c_str ());

		CloseHandle (CreateThread (NULL, 0, DownloadBkCtrlThreadProc,(LPVOID)appid, 0, &dwThreadID)); 
		
		return S_OK ;
	}
	else if (dispIdMember == DISPID_CHANGEORDER_FAV &&  (wFlags & DISPATCH_METHOD))
	{
		CBankData* pBankData = CBankData::GetInstance ();
		std::string strBkID = OLE2A(pDispParams->rgvarg[2].bstrVal);
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"������˳��");
		pBankData->ChangeOrder ((char*)strBkID.c_str (), pDispParams->rgvarg[1].intVal, pDispParams->rgvarg[0].intVal);
	}
	else if (dispIdMember == DISPID_DELETE_FAVORITE && (wFlags & DISPATCH_METHOD))
	{
		//ɾ���ղ�
		CBankData* pBankData = CBankData::GetInstance();
		if (pDispParams->rgvarg[0].vt != VT_BSTR)
			return S_OK;
		std::string strBkID = OLE2A(pDispParams->rgvarg[0].bstrVal);
		RemoveUserFavBank(strBkID); // �Ƴ�

		CBkInfoDownloadManager::GetInstance()->CancleDownload((char*)strBkID.c_str());
		pBankData->DeleteFav((char*)strBkID.c_str());

		CListManager::_()->DeleteAFavBank(strBkID);//
		CListManager::_()->UpdateHMac ();
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"ɾ��һ���ղ�");

		return S_OK ;
	}
	else if (dispIdMember == DISPID_GET_FAVORITE && (wFlags & DISPATCH_METHOD))
	{
		//��ȡ�ղ�
		std::string strFav;

		CBankData* pBankData = CBankData::GetInstance();
		if (pBankData->GetFav(strFav))
		{
			wstring wstrFav = AToW(strFav, 936);
			if (pVarResult != NULL)
			{
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(wstrFav.c_str()));
			}

			return S_OK;
		}
		else
		{
			return E_NOTIMPL;
		}
		// ������Ҫ�ں���������֮����������ղص�״̬
	}
	else if (dispIdMember == DISPID_ADD_EVENT && (wFlags & DISPATCH_METHOD))
	{
		//����¼�
		wstring wInfo = pDispParams->rgvarg[0].bstrVal;
		std::string ev = WToA(wInfo, 936);

		//std::string ev = OLE2A(pDispParams->rgvarg[0].bstrVal);

		CBankData* pBankData = CBankData::GetInstance();
		int ret = pBankData->AddEvent(ev);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_I4;
			pVarResult->lVal = ret;
		}

		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"����¼�");
		CRecordProgram::GetInstance()->RecordDebugInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, A2W(ev.c_str()));
		return S_OK;
	}
	else if (dispIdMember == DISPID_DELETE_EVENT && (wFlags & DISPATCH_METHOD))
	{
		//ɾ���¼�
		int id = pDispParams->rgvarg[0].intVal;

		CBankData* pBankData = CBankData::GetInstance();
		int ret = pBankData->DeleteEvent(id);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_I4;
			pVarResult->lVal = ret;
		}

		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"ɾ���¼�");

		return S_OK;
	}
	else if (dispIdMember == DISPID_GET_EVENTS && (wFlags & DISPATCH_METHOD))
	{
		//��ȡĳһ�µ������¼�
		int year = pDispParams->rgvarg[1].intVal;
		int month = pDispParams->rgvarg[0].intVal;

		CBankData* pBankData = CBankData::GetInstance();
		std::string ret;
		pBankData->GetEvents(year, month, ret);
		wstring wret = AToW(ret, 936);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)wret.c_str());
		}

		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"��ȡ�¼�");
		CRecordProgram::GetInstance()->RecordDebugInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, A2W (ret.c_str ()));

		return S_OK;
	}
	else if (dispIdMember == DISPID_GET_EVENTSONEDAY && (wFlags & DISPATCH_METHOD))
	{
		//��ȡĳһ�յ������¼�
		int year = pDispParams->rgvarg[2].intVal;
		int month = pDispParams->rgvarg[1].intVal;
		int day = pDispParams->rgvarg[0].intVal;

		CBankData* pBankData = CBankData::GetInstance();
		std::string ret;
		pBankData->GetEventsOneDay(year, month, day, ret);
		wstring wret = AToW(ret, 936);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)wret.c_str());
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_SET_ALARM && (wFlags & DISPATCH_METHOD))
	{
		//�趨��ǰ��������
		wstring wInfo = pDispParams->rgvarg[0].bstrVal;
		std::string alarm = WToA(wInfo, 936);
		
		//std::string alarm = OLE2A(pDispParams->rgvarg[0].bstrVal);
		
		CBankData* pBankData = CBankData::GetInstance();
		int ret = pBankData->SetAlarm(alarm);
		
		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_I4;
			pVarResult->intVal = ret;
		}

		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"��������");
		return S_OK;
	}
	else if (dispIdMember == DISPID_GET_TODAYALARMS && (wFlags & DISPATCH_METHOD))
	{
		//��ȡ��������
		CBankData* pBankData = CBankData::GetInstance();
		std::string ret = pBankData->GetTodayAlarms();
		wstring wret = AToW(ret, 936);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)wret.c_str());
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_SAVE_COUPON && (wFlags & DISPATCH_METHOD))
	{
		//�����Ż�ȯ
		int style = pDispParams->rgvarg[3].intVal;
		std::string id = OLE2A(pDispParams->rgvarg[2].bstrVal);
		std::string strDate = OLE2A(pDispParams->rgvarg[1].bstrVal);
		std::string strName = OLE2A(pDispParams->rgvarg[0].bstrVal);
		
		CBankData* pBankData = CBankData::GetInstance();
		int ret = pBankData->SaveCoupon(style, id, strDate, strName);
		
		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_I4;
			pVarResult->lVal = ret;
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_DELETE_COUPON && (wFlags & DISPATCH_METHOD))
	{
		//ɾ���Ż�ȯ
		int id = pDispParams->rgvarg[0].intVal;

		CBankData* pBankData = CBankData::GetInstance();
		int ret = pBankData->DeleteCoupon(id);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_I4;
			pVarResult->lVal = ret;
		}

	
		return S_OK;
	}
	else if (dispIdMember == DISPID_GET_COUPONS && (wFlags & DISPATCH_METHOD))
	{
		//��ȡ�����Ż�ȯ
		CBankData* pBankData = CBankData::GetInstance();
		std::string ret = pBankData->GetCoupons();

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(ret.c_str()));
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_GET_APPDATA && (wFlags & DISPATCH_METHOD))
	{
		//��ȡAPPDATA����·��
		TCHAR szDataPath[MAX_PATH + 1];
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szDataPath);
		_tcscat_s(szDataPath, _T("\\MoneyHub\\"));
		WCHAR szAppDataPath[MAX_PATH + 1];
		ExpandEnvironmentStringsW(szDataPath, szAppDataPath, MAX_PATH);
		
		CStringW strPath = _T("file:///");
		strPath += szDataPath;
		strPath.Replace('\\', '/');

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)strPath);	
		}
		else
		{
			CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"��ȡAPPDATAʧ��");
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_PRINT_HTML && (wFlags & DISPATCH_METHOD))
	{
		//������IE�н��д�ӡ
		/*
		TCHAR szDataPath[MAX_PATH + 1];
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szDataPath);
		_tcscat_s(szDataPath, _T("\\MoneyHub\\Coupons\\"));
		WCHAR szAppDataPath[MAX_PATH + 1];
		ExpandEnvironmentStringsW(szDataPath, szAppDataPath, MAX_PATH);

		CStringW strUrl = _T("file:///");
		strUrl += szDataPath;
		strUrl.Replace('\\', '/');

		CString strId;
		strId.Format(_T("%d"), pDispParams->rgvarg[0].intVal);

		strUrl += strId + _T(".htm");
		*/

		CString strUrl = pDispParams->rgvarg[0].bstrVal;
		CListManager::PrintWebPage(strUrl);

		return S_OK;
	}
	else if (dispIdMember == DISPID_SEND_VISITRECORD && (wFlags & DISPATCH_METHOD))
	{
		// �������ʼ�¼
		std::string strURL = OLE2A(pDispParams->rgvarg[0].bstrVal);
		
		CUserBehavior::GetInstance()->Action_ProgramNavigate(strURL);
		// �ڱ��ؼ�¼
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, A2W(strURL.c_str()));

		pVarResult = NULL;

		return S_OK;
	}
	else if (dispIdMember == DISPID_GET_ENVIRONMENT && (wFlags & DISPATCH_METHOD))
	{
		//��ȡ��������
		WCHAR ret[1024] = { 0 };
		ExpandEnvironmentStringsW(pDispParams->rgvarg[0].bstrVal, ret, sizeof(ret));
		
		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)ret);
		}

		return S_OK;
	}

	else if (dispIdMember == DISPID_REFRESH_PAGE && (wFlags & DISPATCH_METHOD))
	{
		// ˢ��ָ��ҳ��
		int nPage = pDispParams->rgvarg[0].intVal;

		RefreshPage(nPage);	

		pVarResult = NULL;

		return S_OK;
	}
	// ���host��ַ
	else if (dispIdMember == DISPID_HOST_NAME && (wFlags & DISPATCH_METHOD))
	{
		// ˢ��ָ��ҳ��
		std::string strURL = OLE2A(pDispParams->rgvarg[0].bstrVal);

		wstring url;
		if( strURL == "web" )
			url = CHostContainer::GetInstance()->GetHostName(kWeb);
		else if( strURL == "download" )
			url = CHostContainer::GetInstance()->GetHostName(kDownload);
		else if( strURL == "cloud" )
			url = CHostContainer::GetInstance()->GetHostName(kFeedback);
		else if( strURL == "adv" )
			url = CHostContainer::GetInstance()->GetHostName(kAdv);
		else if( strURL == "benefit" )
			url = CHostContainer::GetInstance()->GetHostName(kBenefit);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)url.c_str());			
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_QUERY_SQL && (wFlags & DISPATCH_METHOD))
	{
		std::string strSQL;
		std::string strDbName;
		if (pDispParams->cArgs == 2)
		{
			strSQL = OLE2A(pDispParams->rgvarg[1].bstrVal);
			strDbName = OLE2A(pDispParams->rgvarg[0].bstrVal);
		}
		else
			strSQL = OLE2A(pDispParams->rgvarg[0].bstrVal);

		// ��ѯSQL���

		CBankData* pBankData = CBankData::GetInstance();
		std::string ret = pBankData->QuerySQL(strSQL, strDbName);
		wstring wresult = AToW(ret, 936);//��������ת�� 936Ϊ�й�

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)wresult.c_str());
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_EXECUTE_SQL && (wFlags & DISPATCH_METHOD))
	{
		if (pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			// ִ��SQL���
			wstring wInfo = pDispParams->rgvarg[0].bstrVal;
			std::string strSQL = WToA(wInfo, 936);

			CBankData* pBankData = CBankData::GetInstance();
			int ret = pBankData->ExecuteSQL(strSQL);

			if (pVarResult != NULL)
			{
				pVarResult->vt = VT_I4;
				pVarResult->lVal = ret;
			}
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_SHOWWAITWINDOW && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			string isShow;
			isShow = OLE2A(pDispParams->rgvarg[0].bstrVal);

			if(isShow == "true")
			{
				DWORD dw = 0;
				HANDLE hThread = CreateThread(NULL, 0, _threadShowJSInfoDLG, 0, 0, &dw);
				int i = 0;
				while(i < 20)
				{
					if(g_hJSWndInfoDlg != NULL)
						break;
					Sleep(200);
					i ++;
				}
			}
			else if(isShow == "tshow")
			{
				DWORD dw = 0;
				HANDLE hThread = CreateThread(NULL, 0, _threadShowJSInfoDLG, (LPVOID)1, 0, &dw);
				int i = 0;
				while(i < 20)
				{
					if(g_hJSWndInfoDlg != NULL)
						break;
					Sleep(200);
					i ++;
				}
			}
			else if(isShow == "false")
				_endShowJSInfoDLG();
			else if(isShow == "thide")
				_endShowJSInfoDLG();

			
		}
		return S_OK;
	}
	else if (dispIdMember == DISPID_WRITE_LOG && (wFlags & DISPATCH_METHOD))
	{
#ifdef _DEBUG
		//EnterCriticalSection(&CExternalDispatchImpl::m_cs);
		if(pDispParams->rgvarg[0].vt != VT_BSTR && pDispParams->rgvarg[0].vt != VT_I4)
		{
			MessageBox(NULL, L"��־д�����ʹ���", L"�ƽ��", MB_OK);
			return S_FALSE;
		}
		string info;
		if(pDispParams->rgvarg[0].vt == VT_I4)
		{
			CHAR tp[256] = {0};
			sprintf_s(tp , 256, "%d", pDispParams->rgvarg[0].intVal);
			info = tp;
		}
		else
		{
			wstring wInfo = pDispParams->rgvarg[0].bstrVal;
			info = WToA(wInfo, 936);
		}
		if(m_logHandle == NULL)
		{
			WCHAR szAppDataPath[MAX_PATH + 1];
			ExpandEnvironmentStringsW(L"%APPDATA%\\MoneyHub\\JSLog.txt", szAppDataPath, MAX_PATH);

			m_logHandle = CreateFileW(szAppDataPath,GENERIC_WRITE | GENERIC_READ, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
			if(m_logHandle == INVALID_HANDLE_VALUE)
			{
				m_logHandle = NULL;
			}
		}
		if(m_logHandle != NULL)
		{
			SetFilePointer(m_logHandle, 0, NULL, FILE_END);
			DWORD wl;
			if(!WriteFile(m_logHandle ,(LPVOID)info.c_str(),(info.length()), &wl, NULL))
			{
				WriteFile(m_logHandle ,L"\r\n",4, &wl, NULL);
				//LeaveCriticalSection(&CExternalDispatchImpl::m_cs);
				return S_FALSE;
			}
		}
		//LeaveCriticalSection(&CExternalDispatchImpl::m_cs);
#endif
		return S_OK;
	}
	else if (dispIdMember == DISPID_GET_XML_DATA && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[3].vt == VT_BSTR && pDispParams->rgvarg[2].vt == VT_BSTR && pDispParams->rgvarg[1].vt == VT_BSTR && pDispParams->rgvarg[0].vt == VT_BSTR)
		{

			// ȡ������ͳ�ƽ��
			std::string strStartDate = OLE2A(pDispParams->rgvarg[3].bstrVal);
			std::string strEndDate = OLE2A(pDispParams->rgvarg[2].bstrVal);
			std::string strDataSeries = OLE2A(pDispParams->rgvarg[1].bstrVal);
			std::string strChartType= OLE2A(pDispParams->rgvarg[0].bstrVal);

			CBankData* pBankData = CBankData::GetInstance();
			std::wstring strTemp = CA2W(strChartType.c_str());
			CRecordProgram::GetInstance()->RecordDebugInfo(strTemp, 11, L"��ʼ��ȡͼ������(��һ��������ͼ�͵�����)");

			std::string ret = pBankData->GetXMLData(strStartDate, strEndDate, strDataSeries, strChartType);

			CRecordProgram::GetInstance()->RecordDebugInfo(strTemp, 11, L"��ȡͼ�����ݽ���(��һ��������ͼ�͵�����)");
			wstring wresult = AToW(ret, 936);//��������ת�� 936Ϊ�й�

			if (pVarResult != NULL)
			{
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = ::SysAllocString((LPOLESTR)wresult.c_str());
			}
		}
		return S_OK;
	}
	// ΪJS�����ĺ�PHPͨѶ�Ľӿ�
	else if (dispIdMember == DISPID_GET_PHP_DATA && (wFlags & DISPATCH_METHOD))
	{
		// ȡ������ͳ�ƽ��
		std::string strChartType= OLE2A(pDispParams->rgvarg[0].bstrVal);
		CString strURL = CA2W(strChartType.c_str());
		CString strSavePath;

		WCHAR tempPath[MAX_PATH] = { 0 };

		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, tempPath);
		wcscat_s(tempPath, L"\\MoneyHub");
		::CreateDirectoryW(tempPath, NULL);
		wcscat_s(tempPath, L"\\Data");
		::CreateDirectoryW(tempPath, NULL);
		wcscat_s(tempPath, L"\\TempData");
		::CreateDirectoryW(tempPath, NULL);
		wcscat_s(tempPath, L"\\ad.dat");
		DeleteFile(tempPath);

		char szDbPath[1024] = { 0 };
		int srcCount = wcslen(tempPath);
		srcCount = (srcCount > MAX_PATH) ? MAX_PATH : srcCount;
		::WideCharToMultiByte(CP_UTF8, 0, tempPath, srcCount, szDbPath, 1024, NULL,FALSE);
		std::string strTp = szDbPath;

		strSavePath = CA2W(strTp.c_str());
		CDownloadAndSetupThread cDlThread;
		cDlThread.DownLoadInit(L"fffff", strURL, strSavePath, "fffff");
		cDlThread.DownLoadData(); // ִ����������

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)strSavePath);			
		}


		return S_OK;
	}

	// ΪJS���ò���
	else if (dispIdMember == DISPID_SET_JS_PARAM && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[1].vt == VT_BSTR && pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			std::string strParamName = OLE2A(pDispParams->rgvarg[1].bstrVal);
			std::string strParamValue = OLE2A(pDispParams->rgvarg[0].bstrVal);

			if (strParamName.length() <= 0)
				return S_OK;

			std::map<std::string, std::string>::const_iterator cstIt;
			cstIt = m_mapParam.find(strParamName); // �����Ƿ��Ѿ�����
			if (cstIt != m_mapParam.end())
			{
				// �Ѿ�����
				m_mapParam.erase(cstIt);
			}

			m_mapParam.insert(std::make_pair(strParamName, strParamValue));
			if (pVarResult != NULL)
			{
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = ::SysAllocString((LPOLESTR)L"1");// ����ɹ�
			}
		}
		return S_OK;
	}

	// ΪJS��ȡ����
	else if (dispIdMember == DISPID_GET_JS_PARAM && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			std::string strParamName = OLE2A(pDispParams->rgvarg[0].bstrVal);

			std::map<std::string, std::string>::const_iterator cstIt;
			cstIt = m_mapParam.find(strParamName);
			if (cstIt != m_mapParam.end())
			{
				if (pVarResult != NULL)
				{
					// ���ز���ֵ
					pVarResult->vt = VT_BSTR;
					pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(cstIt->second.c_str()));			
				}
			}
		}
		return S_OK;
	}

	else if (dispIdMember == DISPID_GET_SIZE && (wFlags & DISPATCH_METHOD))
	{
		if (s_rectClient.Width() == 0 || s_rectClient.Height() == 0)
			::GetClientRect(m_hFrame[0], &s_rectClient); 
		//HWND   hTop = ::FindWindowExW(::GetDesktopWindow(),NULL,L"Progman",NULL);//
		//int x = 0, y = 0;
		//if(hTop)
		//{
		//	HWND hTopc = ::FindWindowEx(hTop, NULL, L"SHELLDLL_DefView",NULL);
		//	if(hTopc)
		//	{
		//		HWND hTopb = ::FindWindowEx(hTopc, NULL, L"SysListView32",NULL);
		//		if(hTopb)
		//		{
		//			::GetWindowRect(hTopb,&rc1);
		//			if(IsVista())//Vista��Win7��λ�ú�XP�µĲ�һ��
		//			{
		//				x = rc1.right - 20;// - rc.Width()
		//				y = rc1.bottom - 20;// - rc.Height()
		//			}
		//			else
		//			{
		//				x = rc1.right - 20;// - rc.Width()
		//				y = rc1.bottom;// - rc.Height()
		//			}
		//		}
		//	}				
		//}
		//if(x == 0 || y == 0)
		//{
		//	RECT wkrc;
		//	SystemParametersInfo(SPI_GETWORKAREA, 0, &wkrc, 0);
		//	x = (wkrc.right - wkrc.left) - 20;// - rc.Width()
		//	y = (wkrc.bottom - wkrc.top);// - rc.Height()
		//}

		//int nX = ::GetSystemMetrics(SM_CXSCREEN);
		CString strTemp;
		strTemp.Format(L"%dx%d", s_rectClient.right, s_rectClient.bottom);
		if (pVarResult != NULL)
		{
			// ���ز���ֵ
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)strTemp);			
		}
		return S_OK;
	}

	//else if (dispIdMember == DISPID_GET_HEIGHT && (wFlags & DISPATCH_METHOD))
	//{
	//	//int nY = ::GetSystemMetrics(SM_CYSCREEN);
	//	//CString strTemp;
	//	//strTemp.Format(L"%d", nY);
	//	//if (pVarResult != NULL)
	//	//{
	//	//	// ���ز���ֵ
	//	//	pVarResult->vt = VT_BSTR;
	//	//	pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)strTemp);			
	//	//}

	//	CRect rc1;//rc,
	//	//::GetClientRect(NULL, &rc); 
	//	HWND   hTop = ::FindWindowExW(::GetDesktopWindow(),NULL,L"Progman",NULL);//
	//	int x = 0, y = 0;
	//	if(hTop)
	//	{
	//		HWND hTopc = ::FindWindowEx(hTop, NULL, L"SHELLDLL_DefView",NULL);
	//		if(hTopc)
	//		{
	//			HWND hTopb = ::FindWindowEx(hTopc, NULL, L"SysListView32",NULL);
	//			if(hTopb)
	//			{
	//				::GetWindowRect(hTopb,&rc1);
	//				if(IsVista())//Vista��Win7��λ�ú�XP�µĲ�һ��
	//				{
	//					x = rc1.right - 20;// - rc.Width()
	//					y = rc1.bottom - 20;// - rc.Height()
	//				}
	//				else
	//				{
	//					x = rc1.right - 20;// - rc.Width()
	//					y = rc1.bottom;// - rc.Height()
	//				}
	//			}
	//		}				
	//	}
	//	if(x == 0 || y == 0)
	//	{
	//		RECT wkrc;
	//		SystemParametersInfo(SPI_GETWORKAREA, 0, &wkrc, 0);
	//		x = (wkrc.right - wkrc.left) - 20;// - rc.Width()
	//		y = (wkrc.bottom - wkrc.top);// - rc.Height()
	//	}

	//	//int nX = ::GetSystemMetrics(SM_CXSCREEN);
	//	CString strTemp;
	//	strTemp.Format(L"%d", y);
	//	if (pVarResult != NULL)
	//	{
	//		// ���ز���ֵ
	//		pVarResult->vt = VT_BSTR;
	//		pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)strTemp);			
	//	}
	//	return S_OK;
	//}

	else if (dispIdMember == DISPID_GET_BILL_LIST && (wFlags & DISPATCH_METHOD))
	{
		// ��ȡ�˵�	
		int id = 0;
		if(pDispParams->rgvarg[3].vt == VT_BSTR)
		{
			if(pDispParams->rgvarg[3].bstrVal != NULL)
				id = atoi(OLE2A(pDispParams->rgvarg[3].bstrVal));// �˻�id
		}
		else if(pDispParams->rgvarg[3].vt == VT_I4)
			id = pDispParams->rgvarg[3].intVal;

		std::string aid;
		if(pDispParams->rgvarg[2].bstrVal != NULL)
			aid = OLE2A(pDispParams->rgvarg[2].bstrVal);//������id

		int type = 0;
		if(pDispParams->rgvarg[1].vt == VT_BSTR)
		{
			if(pDispParams->rgvarg[1].bstrVal != NULL)
				type = atoi(OLE2A(pDispParams->rgvarg[1].bstrVal));// �˻�����
		}
		else if(pDispParams->rgvarg[1].vt == VT_I4)
			type = pDispParams->rgvarg[1].intVal;// �˻�����

		string keyInfo;
		if(pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[0].bstrVal != NULL)
		{
			keyInfo = OLE2A(pDispParams->rgvarg[0].bstrVal);
		}

		CString strURL;
		bool isBeginStep;
		wstring url = CBillUrlManager::GetInstance()->GetBillUrl(aid, type, 1, isBeginStep);
		/*if (aid == "a003")
			url.replace(url.find(L"//vip."), 6, L"//mybank.");*/
		if(url.size() < 5)
		{
			MessageBoxW(NULL, L"��ַ�����޷�����", L"�����˵�", MB_OK);
			return S_OK;
		}
		HWND hChildFrame = ::GetExistWindow(g_hMainFrame, (LPCTSTR)url.c_str(), true);
		if(hChildFrame != NULL)
		{
			MessageBoxW(NULL, L"�Ѿ����ڵ����˵�����رջ���ɵ����˵�ҳ���������һ���˵�����", L"�����˵�", MB_OK);
			return S_OK;
		}

		hChildFrame = ::CreateBillPage(g_hMainFrame, url.c_str(), true);
		if(hChildFrame != NULL){

			HWND hAxControl = NULL ;
			for (int i = 0; i < 400; ++i)
			{
				hAxControl = (HWND)::SendMessage(hChildFrame, WM_ITEM_GET_AX_CONTROL_WND, 0, 0);
				if (hAxControl)
					break ;
				Sleep(10) ;
			}
			
			if(hAxControl)
			{
				BillData* pBData = new BillData;
				if(pBData)
				{
					pBData->aid = aid;
					pBData->accountid = id; 
					pBData->type = type;
					pBData->tag = keyInfo;
				}

				PostMessageW(hAxControl, WM_AX_GET_BILL, 0, (LPARAM)pBData);
			}
			else
				MessageBox(NULL, L"ץȡ�˵�ʧ��", L"�˵�����", MB_OK);

		}
		return S_OK;
	}

	return E_NOTIMPL ; 
}

void CExternalDispatchImpl::RefreshPage(int nPage)
{
	if(m_hFrame[nPage -1] != NULL)
	{
		switch(nPage)
		{
			case 1:
				PostMessageW(m_hFrame[nPage -1], WM_AX_FRAME_SETALARM, 0, 0);
				break;
			case 2:
				PostMessageW(m_hFrame[nPage -1], WM_AX_TOOLS_CHANGE, 0, 0);
				break;
			//case 3:
				//PostMessageW(m_hFrame[nPage -1], WM_AX_COUPON_CHANGE, 0, 0);
				break;
		}
	}
}

// ��¼�û��ղص�����
bool CExternalDispatchImpl::AddUserFavBank(const std::string& strBkID)
{
	ATLASSERT(!strBkID.empty());
	if (strBkID.empty())
		return false;

	if (IsInUserFavBankList(strBkID))
		return false;

	m_sstrVerctor.push_back(strBkID);
	return true;
}

// �Ƴ��û��ղص�����
bool CExternalDispatchImpl::RemoveUserFavBank(const std::string& strBkID)
{
	ATLASSERT(!strBkID.empty());
	if (strBkID.empty())
		return false;

	if (!IsInUserFavBankList(strBkID))
		return false;

	// �Ƴ�
	m_sstrVerctor.remove(strBkID);
	return true;
}

bool CExternalDispatchImpl::IsInUserFavBankList(const std::string& strBkID)
{
	std::list<std::string>::const_iterator cstIt;
	for(cstIt = m_sstrVerctor.begin(); cstIt != m_sstrVerctor.end(); cstIt ++)
	{
		std::string strTp = *cstIt;
		if (strTp == strBkID)
			break;
	}

	if (cstIt != m_sstrVerctor.end())
		return true;

	return false;
}

bool CExternalDispatchImpl::IsVista()
{
	OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
	BOOL bRet = ::GetVersionEx(&ovi);
	return ((bRet != FALSE) && (ovi.dwMajorVersion >= 6));
}