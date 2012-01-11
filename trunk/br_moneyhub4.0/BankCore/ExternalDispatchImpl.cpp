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
#include "../Utils/SecurityCache/comm.h"
#include "../Utils/HardwareID/genhwid.h" // ��ȡӲ��ID
#include "../Utils/SN/SNManager.h" // ��ȡSN
#include "../Utils/CryptHash/base64.h"
#include "../Utils/PublicInterface/PublicInterface.h"
#include "AxUI.h"

#define CMD_GET_UT				L"GetUT"
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
#define CMD_USER_LOAD			L"UserLoad" // �û���¼�ӿ�
#define CMD_USER_REGEDIT		L"UserRegedit" // �û�ע��ӿ�
#define CMD_USER_CHECK			L"UserMailCheck" // ����У��ӿ�
#define CMD_USER_AUTO_DLG		L"AutoDialog" // �򿪻�ر�ָ���ĶԻ���ӿ�
//#define CMD_USER_AUTO_LOAD		L"AutoLoad" // �Զ���¼
//#define CMD_SHELL_EXPLORER		L"ShellExplorer" // ��ת����ҳ
#define CMD_CHANGE_WINDOW_NAME	L"ChangeWindowName"
#define CMD_CHANGE_MAIL_PWD		L"ChangeMailOrPwd" // �޸����������
#define CMD_GET_CUR_USERID		L"GetCurrentUserID" // ��ȡ��ǰ��¼�û���USID��δ��¼ʱ���ء�Guest��
#define CMD_SET_GUIDE_INFO_PM	L"SetRegGuideInfoParam" // ����ע�����Ƿ���ʾ����
#define CMD_GET_CUR_USERSTATUS	L"GetCurrentSettingStatus" // ��ȡ�û����õ�״̬�����룬���䣬��
#define CMD_QUERY_USER_LOAD		L"QuitUserLoad"
#define CMD_READ_ETK			L"ReadETK"
#define CMD_DEL_USERDB_BY_ID	L"DeleteUserDbByID"
#define CMD_SEND_MAIL_FOR_OPT	L"SendMailForOPT" // ֪ͨ��������ָ�������䷢��OPT
#define CMD_SEND_RECEIVED_OPT	L"SendReceivedOPT" // ������������OPT
#define CMD_INIT_NEW_PWD		L"InitNewPassword" // ��������

#define CMD_SHOWWAITWINDOW		L"ShowWaitWindow"

#define MAIL_CHANGE_SUCC		"72" // �����޸ĳɹ�
#define PWD_CHANGE_SUCC			"71" // �����޸ĳɹ�
#define PWD_CHANGE_MAIL_ERR		"73" // �޸�����ʱmail����
#define PWD_CHANGE_STOKEN_ERR	"65" // stoken����
#define USER_REGE_SUCC			"41" // �û�ע��ɹ�
#define MANU_LOAD_SUCC			"51" // �ֶ���¼�ɹ�
#define CURMAIL_CAN_NOT_FIND	"52" // ��ǰ����ʧЧ
#define MANU_LOAD_PWD_ERR		"53" // �����������
#define MAUN_LOAD_LAN_FIRST		"101" // �״α��ص�¼
#define MANU_LOAD_LAN_ERR		"102" // �ֶ����ص�¼ʧ��
#define MANU_LOAD_LAN_SUCC		"103" // �ֶ����ص�¼�ɹ�
#define MANU_LOAD_PWD_OUTOFTIME "104" // �ֶ���¼ʱ�����ѹ���
#define REGISTER_FREQUENTLY		"105" // ע��̫Ƶ��
#define CHECK_OPT_SUCC			"228" // У��OPT�ɹ�
#define INIT_PASSWORD_SUCC		"238" // ��������ɹ�
#define SEND_MAIL_FREQUENTLY	"241" // ����OPT�ʼ�̫��Ƶ��
#define USER_DB_PWD_LEN			32 // �û����ݿ����볤��


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
#define DISPID_USER_LOAD		12428
#define DISPID_USER_REGEDIT		12429
#define DISPID_USER_CHECK		12430
#define DISPID_USER_AUTO_DLG	12431
//#define DISPID_USER_AUTO_LOAD	12432
//#define DISPID_SHELL_EXPLORER	12433
#define DISPID_CHANGE_WIN_NAME	12434
#define DISPID_CHANGE_MIAL_PWD	12435
#define DISPID_GET_CUR_USERID	12436
#define DISPID_SET_GUIDE_INFO	12437
#define DISPID_GET_CUR_STATUS	12438
#define DISPID_QUERY_USER_LOAD	12439
#define DISPID_READ_ETK			12440
#define DISPID_DEL_USERDB_BY_ID 12441
#define DISPID_SEND_MAIL_FOROPT 12442
#define DISPID_SEND_RECV_OPT	12443
#define DISPID_INIT_NEW_PWD		12444

#define DISPID_GET_BILL_LIST	12500

#define DISPID_WRITE_LOG		12600
#define DISPID_SHOWWAITWINDOW	12610
#define DISPID_GET_UT			12611//js��ȡut


extern HWND g_hMainFrame;

HWND CExternalDispatchImpl::m_hFrame[3] = {NULL, NULL, NULL};
HWND CExternalDispatchImpl::m_hAxui = 0;
HANDLE CExternalDispatchImpl::m_logHandle = NULL;
CRect CExternalDispatchImpl::s_rectClient = CRect(0, 0, 0, 0);
std::list<std::string> CExternalDispatchImpl::m_sstrVerctor;
std::map<std::string, std::string> CExternalDispatchImpl::m_mapParam;
CTime CExternalDispatchImpl::m_sLastRegTime;
//CTime CExternalDispatchImpl::m_sLastFindMailTime;
bool CExternalDispatchImpl::m_sbUpdateSynchroBtn = false;
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
		else if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_UT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_UT;
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
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_USER_LOAD, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_USER_LOAD;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_USER_REGEDIT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_USER_REGEDIT;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_USER_CHECK, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_USER_CHECK;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_USER_AUTO_DLG, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_USER_AUTO_DLG;
		}
		/*else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_USER_AUTO_LOAD, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_USER_AUTO_LOAD;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SHELL_EXPLORER, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SHELL_EXPLORER;
		}*/
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SEND_MAIL_FOR_OPT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SEND_MAIL_FOROPT;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SEND_RECEIVED_OPT, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SEND_RECV_OPT;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_INIT_NEW_PWD, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_INIT_NEW_PWD;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_READ_ETK, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_READ_ETK;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_DEL_USERDB_BY_ID, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_DEL_USERDB_BY_ID;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_CHANGE_WINDOW_NAME, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_CHANGE_WIN_NAME;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_CHANGE_MAIL_PWD, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_CHANGE_MIAL_PWD;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_CUR_USERID, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_CUR_USERID;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_SET_GUIDE_INFO_PM, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_SET_GUIDE_INFO;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_GET_CUR_USERSTATUS, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_GET_CUR_STATUS;
		}
		else if(CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE | NORM_IGNOREWIDTH, CMD_QUERY_USER_LOAD, -1, (TCHAR *)rgszNames[i], -1))
		{
			rgDispId[i] = DISPID_QUERY_USER_LOAD;
		}
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
	else if (dispIdMember == DISPID_GET_UT &&  (wFlags & DISPATCH_METHOD))
	{
		CBankData* pBankData = CBankData::GetInstance ();
		INT64 nUT = pBankData->GetNewUT();
		if (pVarResult != NULL)
		{
			CString strTp;
			strTp.Format(L"%I64d", nUT);
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)strTp);
		}
		return S_OK;
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
		INT64 ret = pBankData->AddEvent(ev);
		wchar_t wid[ 256 ] = {0};
		swprintf_s(wid, 256, L"%I64d", ret);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(wid));;
		}

		if (!m_sbUpdateSynchroBtn && ret > 0)
		{
			ChangeCurUserSynchroStatus();
			/*::SendMessage(g_hMainFrame, WM_NOTIFYUI_SYNCHRO_BTN_CHANGE, 0, 0);
			string strSQL = "update datUserInfo set needsynchro = 1 where userid = '";
			strSQL += CBankData::GetInstance()->m_CurUserInfo.struserid;
			strSQL += "'";
			CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");
			m_sbUpdateSynchroBtn = true;*/
		}


		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"����¼�");
		CRecordProgram::GetInstance()->RecordDebugInfo(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, A2W(ev.c_str()));
		return S_OK;
	}
	else if (dispIdMember == DISPID_DELETE_EVENT && (wFlags & DISPATCH_METHOD))
	{
		USES_CONVERSION;
		//ɾ���¼�		
		INT64 id = 0;
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			string sid = W2A(pDispParams->rgvarg[0].bstrVal);
			id = _atoi64(sid.c_str());
		}
		else
			return S_OK;
		CBankData* pBankData = CBankData::GetInstance();
		int ret = pBankData->DeleteEvent(id);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_I4;
			pVarResult->lVal = ret;
		}

		
		if (!m_sbUpdateSynchroBtn && ret > 0)
		{
			ChangeCurUserSynchroStatus();
			/*::SendMessage(g_hMainFrame, WM_NOTIFYUI_SYNCHRO_BTN_CHANGE, 0, 0);
			string strSQL = "update datUserInfo set needsynchro = 1 where userid = '";
			strSQL += CBankData::GetInstance()->m_CurUserInfo.struserid;
			strSQL += "'";
			CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");
			m_sbUpdateSynchroBtn = true;*/
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
			url = CHostContainer::GetInstance()->GetHostName(kJsWeb);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)url.c_str());			
		}

		return S_OK;
	}
	else if (dispIdMember == DISPID_QUERY_SQL && (wFlags & DISPATCH_METHOD))
	{
		// ִ�в�ѯSQL���
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
		wstring wInfo;
		std::string strDbName;
		if (pDispParams->cArgs == 2)
		{
			wInfo = pDispParams->rgvarg[1].bstrVal;
			strDbName = OLE2A(pDispParams->rgvarg[0].bstrVal);
		}
		else
			wInfo = pDispParams->rgvarg[0].bstrVal;;

		// ִ��SQL���
		string strSQL = WToA(wInfo, 936);
		OutputDebugStringA(strSQL.c_str());
		CBankData* pBankData = CBankData::GetInstance();
		int ret = pBankData->ExecuteSQL(strSQL, strDbName);

		if (pVarResult != NULL)
		{
			pVarResult->vt = VT_I4;
			pVarResult->lVal = ret;
		}

		// ������û������ݸ���ʱ����Ҫͬ��
		if ("DataDB" != strDbName && "SysDB" != strDbName)
			if (!m_sbUpdateSynchroBtn && ret > 0)
			{
				// ������Ʋ�Ʒ
				if (strSQL.find("tbProductChoice") != string::npos)
					return S_OK;

				ChangeCurUserSynchroStatus();
				/*::SendMessage(g_hMainFrame, WM_NOTIFYUI_SYNCHRO_BTN_CHANGE, 0, 0);
				strSQL = "update datUserInfo set needsynchro = 1 where userid = '";
				strSQL += CBankData::GetInstance()->m_CurUserInfo.struserid;
				strSQL += "'";
				CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");
				m_sbUpdateSynchroBtn = true;*/
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
		//if(pDispParams->rgvarg[0].vt != VT_BSTR && pDispParams->rgvarg[0].vt != VT_I4)
		//{
		//	MessageBox(NULL, L"��־д�����ʹ���", L"�ƽ��", MB_OK);
		//	return S_FALSE;
		//}
		//string info;
		//if(pDispParams->rgvarg[0].vt == VT_I4)
		//{
		//	CHAR tp[256] = {0};
		//	sprintf_s(tp , 256, "%d", pDispParams->rgvarg[0].intVal);
		//	info = tp;
		//}
		//else
		//{
		//	wstring wInfo = pDispParams->rgvarg[0].bstrVal;
		//	info = WToA(wInfo, 936);
		//}
		//if(m_logHandle == NULL)
		//{
		//	WCHAR szAppDataPath[MAX_PATH + 1];
		//	ExpandEnvironmentStringsW(L"%APPDATA%\\MoneyHub\\JSLog.txt", szAppDataPath, MAX_PATH);

		//	m_logHandle = CreateFileW(szAppDataPath,GENERIC_WRITE | GENERIC_READ, 
		//		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
		//	if(m_logHandle == INVALID_HANDLE_VALUE)
		//	{
		//		m_logHandle = NULL;
		//	}
		//}
		//if(m_logHandle != NULL)
		//{
		//	SetFilePointer(m_logHandle, 0, NULL, FILE_END);
		//	DWORD wl;
		//	if(!WriteFile(m_logHandle ,(LPVOID)info.c_str(),(info.length()), &wl, NULL))
		//	{
		//		WriteFile(m_logHandle ,L"\r\n",4, &wl, NULL);				
		//		return S_FALSE;
		//	}
		//}
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
		cDlThread.DownLoadInit(L"fffff", strURL, "fffff");
		cDlThread.DownLoadDataWithFile(strSavePath); // ִ����������

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

			AddJSParam(strParamName.c_str(), strParamValue.c_str());

			//std::map<std::string, std::string>::const_iterator cstIt;
			//cstIt = m_mapParam.find(strParamName); // �����Ƿ��Ѿ�����
			//if (cstIt != m_mapParam.end())
			//{
			//	// �Ѿ�����
			//	m_mapParam.erase(cstIt);
			//}

			//m_mapParam.insert(std::make_pair(strParamName, strParamValue));

			// ��ʵ�ô���û�д��ڵı�Ҫ
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

	// �û���¼�ӿ�
	else if (dispIdMember == DISPID_USER_LOAD && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[1].vt == VT_BSTR && pDispParams->rgvarg[2].vt == VT_BSTR)
		{
			string strAutoLoad = OLE2A(pDispParams->rgvarg[0].bstrVal); // Ϊtrue����false
			string strPassword = OLE2A(pDispParams->rgvarg[1].bstrVal);
			string strMail = OLE2A(pDispParams->rgvarg[2].bstrVal);
			string strStoken, strEDEK, strUserID, strMailVerify;
			int nLanLoad = -1;

			USERSTATUS emStatus = emNotLoad; // �û���¼״̬��δ��¼�����ص�¼�������¼��

			// ��������KEK
			char pKek[33] = {0};
			UserDataASH256((unsigned char*)strPassword.c_str(), strPassword.length(), (unsigned char*)pKek);
			string strTp;
			FormatHEXString(pKek, 32, strTp);

			// ���췢�͵��������˵Ĳ���
			string strParam = "xml=" + strMail + MY_PARAM_END_TAG;
			strParam += strTp;
			strParam += MY_PARAM_END_TAG;

			string strHWID = GenHWID2();
			wstring wstrHWID = CA2W(strHWID.c_str());
			string strSN = CSNManager::GetInstance()->GetSN();

			/*strParam += strSN;
			strParam += MY_PARAM_END_TAG;

			strParam += strHWID;
			strParam += MY_PARAM_END_TAG;*/

#define KEY_GUEST_DB_VERSION 7 //�ؼ������ݰ汾��

			char chTemp[1024] = {0};
			DWORD dwRead = 0;
			string strSub;
			CDownloadThread downloadThread;
			downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPManuLogon)).c_str(), (LPSTR)strParam.c_str());
			int nBackVal = downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead);
			if (ERR_SUCCESS != nBackVal || 0 == dwRead)
			{
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"�ֶ�ͨѶʱ�������ͨ��ʧ��:%d",nBackVal));

				nLanLoad = ExcuteLocalLoad(strMail.c_str(), (unsigned char* )pKek, 32, strUserID);

				int nSucc = atoi(MANU_LOAD_LAN_SUCC);
				if(nSucc == nLanLoad)
					emStatus = emUserLocal;
				
			}

			//if () ���Ϊ41���ɹ������43���Ѿ�����
			string strRead = chTemp;
			if (strRead.find(MY_PARAM_END_TAG) != string::npos)
				strSub = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));
			if (strSub.find(MANU_LOAD_SUCC) != string::npos) // ���Ϊ51���ɹ�
			{
				emStatus = emUserNet; // �����¼

				// ��ȡstoken
				string strTag = MY_PARAM_END_TAG;
				strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
				strStoken = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

				strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
				strEDEK = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

				strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
				strUserID = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

				// ��ȡ�ʼ���֤��Ϣ
				strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
				strMailVerify = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));


				// ����UserID��DataDB�з����
				string strSQL = "select userid from datUserInfo where userid = '";
				strSQL += strUserID;
				strSQL += "'";
				string strBack = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");
				if (strBack.find(strUserID) == string::npos) // ������
				{
					string strSubSQL = "insert into datUserInfo(userid, mail, edek) values ('";
					strSubSQL += strUserID;
					strSubSQL += "','";
					strSubSQL += strMail;
					strSubSQL += "','";
					strSubSQL += strEDEK;
					strSubSQL += "')";

					// �¼�һ����¼��DataDB��
					CBankData::GetInstance()->ExecuteSQL(strSubSQL, "DataDB");
				}


				// ���鵱ǰ�û��Ƿ����Զ���¼�û�������ǣ������stoken�����򲻸���
				/*
				strSQL = "select userid from datUserInfo where userid = '";
				strSQL += strUserID;
				strSQL += "' and autoload = 1";
				string strQueryVal = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");*/

				strSQL = "update  datUserInfo set edek = '";
				strSQL += strEDEK;
				if (strAutoLoad == "true")
				{
					// ��stoken���м���
					char chTek[33] = {0};
					string strTekMake = strSN + strHWID;
					UserDataASH256((unsigned char*)strTekMake.c_str(), strTekMake.length(), (unsigned char*)chTek);

					char* pEtk = new char[strStoken.length() + 20];
					memset(pEtk, 0, strStoken.length() + 20);

					int nLen = UserDataASE256E((unsigned char *)strStoken.c_str(), strStoken.length(), (unsigned char *)chTek, (unsigned char*)pEtk);
					FormatHEXString(pEtk, nLen, strTp);
					strSQL += "', etk = '";
					strSQL += strTp;
					delete []pEtk;
				}
				else
				{
					// ���etk
					strSQL += "', etk = '";
				}
				strSQL += "', mail = '";
				strSQL += strMail;
				strSQL += "'";
				if (strAutoLoad == "true")
				{
					// ����ǰ���õ��Զ���¼����
					string strSubSQL = "update datUserInfo set autoload = 0 where autoload = 1";
					CBankData::GetInstance()->ExecuteSQL(strSubSQL, "DataDB");

					strSQL += ",autoload = 1";
				}
				else
				{
					strSQL += ",autoload = 0";
				}
				strSQL += " where userid = '";
				strSQL += strUserID;
				strSQL += "'";

				// ��¼mail,EDEK��DataDB.dat��
				CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");

				char edekVal[33] = {0};
				char keyVal[USER_DB_PWD_LEN + 1] = {0};
				int nBack = 0;
				// ��ʮ�����Ʒ������ʮ��������
				FormatDecVal(strEDEK.c_str(), edekVal, nBack);

				// ��EDEK���н���
				UserDataASE256D((unsigned char*)edekVal, 32, (unsigned char*)pKek, (unsigned char*)keyVal);

				// �����û��Ŀ����óɵ�ǰ���ݿ�,���������
				string strUserDb = strUserID + ".dat";

				// ������ݿⲻ������GUEST����tbAccount��Ϊ��
				if (!CBankData::GetInstance()->IsUserDBExist((LPSTR)strUserDb.c_str()))
				{
					// ��ȡGuestDB�汾
					int nVer = CBankData::GetInstance()->GetGuestDBVersion();
					if (nVer <= KEY_GUEST_DB_VERSION) // 7��4.0�汾��ǰ�����ݿ�
					{
						char chVer[5] = {0};
						itoa(nVer, chVer, 10);
						string strName = chVer;
						strName += "backupGuest";
						// ��Guest��������
						CBankData::GetInstance()->RenameUserDbByID(MONHUB_GUEST_USERID, strName.c_str());
						string strPw;
						int nPwLen = 0;

#ifdef OFFICIAL_VERSION
						strPw = "NCrFT2RIeD0NY2wHOI8W";
						nPwLen = 20;
#endif
						string strGDb = MONHUB_GUEST_USERID;
						strGDb += ".dat";
						// ����һ���¿�
						CBankData::GetInstance()->CreateNewUserDB((LPSTR)strGDb.c_str(), (LPSTR)strPw.c_str(), nPwLen);
					}

					if(CBankData::GetInstance()->IsTableEmptyInGuestDB("tbAccount") && nVer > KEY_GUEST_DB_VERSION)
					{
						// ���ضԻ���
						::PostMessage(g_hMainFrame, WM_AUTO_USER_DLG, MY_TAG_REGISTER_DLG, MY_STATUS_HIDE_DLG);
						int nMesVal = mhMessageBox(g_hMainFrame, L"��֮ǰ��δ��¼״̬�£��ÿ��˻���¼��������Ƿ�Ҫת�Ƶ���ǰ���˻��У���ת�ƺ�,�ÿ��˻��е����ݽ��������", L"��ʾ", MB_YESNO | MB_ICONINFORMATION);
						
						if (IDYES == nMesVal)
						{
							// ֪ͨUI�رյ�ǰ�û���
							::SendMessage(g_hMainFrame, WM_NOTIFYUI_CLOSE_CUR_USER, 0 , 0);
							// ����ÿ�����
							CBankData::GetInstance()->CreateUserDbFromGuestDb((LPSTR)strUserDb.c_str(), (LPSTR)keyVal, USER_DB_PWD_LEN);

							if (!m_sbUpdateSynchroBtn)
							{
								ChangeCurUserSynchroStatus();
							}
						}
						else // ������ÿ�����
							CBankData::GetInstance()->CreateNewUserDB((LPSTR)strUserDb.c_str(), (LPSTR)keyVal, USER_DB_PWD_LEN);
					}
					else // ������ÿ�����
						CBankData::GetInstance()->CreateNewUserDB((LPSTR)strUserDb.c_str(), (LPSTR)keyVal, USER_DB_PWD_LEN);


				}

				// ��������ʱʹ��
#ifndef OFFICIAL_VERSION
				FormatHEXString(keyVal, 32, strTp);
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL,
					CRecordProgram::GetInstance()->GetRecordInfo(L"�û���%s DEK��%s", CA2W(strUserDb.c_str()), CA2W(strTp.c_str())));
#endif

				CBankData::GetInstance()->SetCurrentUserDB((LPSTR)strUserDb.c_str(), keyVal, USER_DB_PWD_LEN);


				// ֪ͨUI���µ�ǰ�û���
				string strSendUI = strUserDb;
				strSendUI += MY_PARAM_END_TAG;
				string strPWD;
				FormatHEXString(keyVal, 32, strPWD);
				strSendUI += strPWD;
				strSendUI += MY_PARAM_END_TAG;
				::SendMessage(g_hMainFrame, WM_SETTEXT, WM_NOTIFYUI_UPDATE_USER_DB, (LPARAM)strSendUI.c_str());


			}

			if (strUserID.length() > 0 && strUserID != MONHUB_GUEST_USERID)
			{
				// ���µ�ǰ�û���Ϣ
				CAxUI::UpdateUserInfo(strStoken.c_str(), strMail.c_str(), strUserID.c_str(), (int)emStatus, false, (char*)strMailVerify.c_str());
			}

			if (pVarResult != NULL)
			{
				// �����ص�¼�Ľ������ת��������JS�ӿ�
				if (-1 != nLanLoad)
				{
					CString strTemp;
					strTemp.Format(L"    %d", nLanLoad);
					strSub = CW2A(strTemp);
				}
				// ���ز���ֵ
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strSub.c_str()));

			}


		}
		return S_OK;
	}
	// �û�ע��ӿ�
	else if (dispIdMember == DISPID_USER_REGEDIT && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[1].vt == VT_BSTR)
		{
			int n = m_sLastRegTime.GetYear() ;
			if (m_sLastRegTime.GetYear() == 1970)
			{
				m_sLastRegTime = CTime::GetCurrentTime();
			}
			else
			{
				CTime tNow = CTime::GetCurrentTime();
				CTimeSpan timeSpan = tNow - m_sLastRegTime;
				int nSec = timeSpan.GetTotalSeconds();
				m_sLastRegTime = tNow;
				if (nSec < 20) // 20S֮��ֻ�ܷ�һ��
				{
					
					// ���ز���ֵ
					pVarResult->vt = VT_BSTR;
					pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(REGISTER_FREQUENTLY));
					return S_OK;
				}
			}

			string strPassword = OLE2A(pDispParams->rgvarg[0].bstrVal);
			string strMail = OLE2A(pDispParams->rgvarg[1].bstrVal);
			string strParam = strMail + MY_PARAM_END_TAG;

			char pKek[33] = {0};
			// ����KEY
			UserDataASH256((unsigned char*)strPassword.c_str(), strPassword.length(), (unsigned char*)pKek);

			string strTp;
			FormatHEXString(pKek, 32, strTp);

			// ���KEK
			strParam += strTp;
			strParam += MY_PARAM_END_TAG;


			unsigned char chValue[USER_DB_PWD_LEN + 1] = {0};
			// �������DEK
			Rand20ByteUCharData(chValue, USER_DB_PWD_LEN);
			
			char pEDEK[33] = {0};
			UserDataASE256E(chValue, USER_DB_PWD_LEN, (unsigned char *)pKek, (unsigned char*)pEDEK); // ��KEK��������EDEK

			FormatHEXString(pEDEK, 32, strTp);
			// ��Ӳ���EDEK
			strParam += strTp;
			strParam += MY_PARAM_END_TAG;

			// ��Ӳ���SN
			/*strParam += CSNManager::GetInstance()->GetSN();
			strParam += MY_PARAM_END_TAG;*/

			// ��Ӳ���HID
			string strHWID = GenHWID2();
			/*strParam += strHWID;
			strParam += MY_PARAM_END_TAG;*/


			string strSend = "xml=" + strParam;

			wstring wstrHWID = CA2W(strHWID.c_str());
			
			CDownloadThread downloadThread;
			downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPRegistration)).c_str(), (LPSTR)strSend.c_str());
			char chTemp[1024] = {0};
			DWORD dwRead = 0;
			string strSub;
			int nBackVal = downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead);
			if (ERR_SUCCESS != nBackVal)
			{
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"�û�ע��ʱ�������ͨ��ʧ��:%d",nBackVal));
			}
			else
			{
				string strRead = chTemp;
				if (strRead.find(MY_PARAM_END_TAG) != string::npos)
					strSub = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

				if (strSub.find(USER_REGE_SUCC) != string::npos) // ���Ϊ41���ɹ�
				{
					// ��ȡuserid
					string strTag = MY_PARAM_END_TAG;
					strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
					string strUserID = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

					string strSQL = "insert into datUserInfo(userid, mail, edek) values ('";
					strSQL += strUserID;
					strSQL += "','";
					strSQL += strMail;
					strSQL += "','";
					strSQL += strTp;
					strSQL += "')";

					// ��¼mail,EDEK��DataDB.dat��
					CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");
					CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"�û�ע��ɹ���");

				}
			}

			if (pVarResult != NULL)
			{
				// ���ز���ֵ
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strSub.c_str()));
				
			}

		}

		return S_OK;
	}
	// �������ӿ�
	else if (dispIdMember == DISPID_USER_CHECK && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			std::string strParam = OLE2A(pDispParams->rgvarg[0].bstrVal);

			if (strParam.length() <= 0)
				return S_OK;

			strParam += MY_PARAM_END_TAG;

			string strSend = "xml=";
			strSend += strParam;

			wstring wstrHWID = CA2W(GenHWID2().c_str());
			

			CDownloadThread downloadThread;
			downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPBeforeRegistration)).c_str(), (LPSTR)strSend.c_str());
			char chTemp[256] = {0};
			DWORD dwRead = 0;
			string strSub;
			int nBackVal = downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead);
			if (ERR_SUCCESS != nBackVal)
			{
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"����У��ʱ�������ͨ��ʧ��:%d",nBackVal));
			}

			string strRead = chTemp;
			if (strRead.find(MY_PARAM_END_TAG) != string::npos)
				strSub = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));
			
			if (pVarResult != NULL)
			{
				// ���ز���ֵ
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strSub.c_str()));
			}

		}
		return S_OK;
	}

	// �򿪻�ر��û��Ի���ӿ�
	else if (dispIdMember == DISPID_USER_AUTO_DLG && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[1].vt == VT_BSTR)
		{
			std::string strParamType = OLE2A(pDispParams->rgvarg[0].bstrVal); // �������� �򿪻��ǹر�
			std::string strParamName = OLE2A(pDispParams->rgvarg[1].bstrVal); // ҳ������ ע�ỹ�ǵ�¼
			int nName = -1, nType = -1;

			if ("load" == strParamName)
				nName = MY_TAG_LOAD_DLG;
			else if ("register" == strParamName)
				nName = MY_TAG_REGISTER_DLG;
			else if ("setting" == strParamName)
				nName = MY_TAG_SETTING_DLG;
			else if ("registerguide" == strParamName)
				nName = MY_TAG_REGISTER_GUIDE;
			else if ("initpwd" == strParamName)
				nName = MY_TAG_INIT_PWD;

			if ("true" == strParamType)
				nType = MY_STATUE_SHOW_DLG;
			else if ("false" == strParamType)
				nType = MY_STATUE_CLOSE_DLG;

			::PostMessage(g_hMainFrame, WM_AUTO_USER_DLG, nName, nType);
		}
		return S_OK;
	}

	//// �Զ���¼
	//else if (dispIdMember == DISPID_USER_AUTO_LOAD && (wFlags & DISPATCH_METHOD))
	//{
	//	if(pDispParams->rgvarg[0].vt == VT_BSTR)
	//	{
	//		std::string strMail = OLE2A(pDispParams->rgvarg[0].bstrVal);

	//	}
	//	return S_OK;
	//}

	//// ����ҳ
	//else if (dispIdMember == DISPID_SHELL_EXPLORER && (wFlags & DISPATCH_METHOD))
	//{
	//	return S_OK;
	//}

	// ֪ͨ����������OPT��ָ����������
	else if (dispIdMember == DISPID_SEND_MAIL_FOROPT && (wFlags & DISPATCH_METHOD))
	{
		string strBack;

		//int n = m_sLastFindMailTime.GetYear() ;
		//if (m_sLastFindMailTime.GetYear() == 1970)
		//{
		//	m_sLastFindMailTime = CTime::GetCurrentTime();
		//}
		//else
		//{
		//	CTime tNow = CTime::GetCurrentTime();
		//	CTimeSpan timeSpan = tNow - m_sLastFindMailTime;
		//	int nSec = timeSpan.GetTotalSeconds();
		//	m_sLastFindMailTime = tNow;
		//	if (nSec < 300) // 300S֮��ֻ�ܷ�һ��
		//	{
		//		// ���ز���ֵ
		//		pVarResult->vt = VT_BSTR;
		//		pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(SEND_MAIL_FREQUENTLY));
		//		return S_OK;
		//	}
		//}

		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{

			std::string strParam = OLE2A(pDispParams->rgvarg[0].bstrVal);

			list<string> listParam;
			listParam.push_back(strParam);
			
			int nBackVal = CommunicateWithServer(kPSendFindMail, listParam, strBack);
			int nLen = strBack.length();
			if (ERR_SUCCESS != nBackVal)
			{
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"֪ͨ����������OPTʱ�������ͨ��ʧ��:%d",nBackVal));
			}

		}

		if (pVarResult != NULL)
		{
			// ���ز���ֵ
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strBack.c_str()));
		}
		return S_OK;
	}
	// ����OPT
	else if (dispIdMember == DISPID_SEND_RECV_OPT && (wFlags & DISPATCH_METHOD))
	{
		string strBack;
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			std::string strParam = OLE2A(pDispParams->rgvarg[0].bstrVal);

			list<string> listParam;
			listParam.push_back(strParam);
			
			int nBackVal = CommunicateWithServer(kPCheckOPT, listParam, strBack);
			if (ERR_SUCCESS != nBackVal)
			{
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"У��OPTʱ�������ͨ��ʧ��:%d",nBackVal));
			}
			
			if (strBack.find(CHECK_OPT_SUCC) != string::npos)
				AddJSParam("my_opt_check_back", strBack.c_str()); // ��¼�·������˷��ص����ݣ�����������ʱʹ��
			else
				AddJSParam("my_opt_check_back", ""); // �����¼����������


		}

		if (pVarResult != NULL)
		{
			int nIndex = strBack.find(MY_PARAM_END_TAG);
			if (nIndex != string::npos)
				strBack = strBack.substr(0, nIndex + 1);
			// ���ز���ֵ
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strBack.c_str()));
		}
		return S_OK;
	}
	// ��������
	else if (dispIdMember == DISPID_INIT_NEW_PWD && (wFlags & DISPATCH_METHOD))
	{
		string strBack;
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			// ������
			std::string strParam = OLE2A(pDispParams->rgvarg[0].bstrVal);
			std::map<std::string, std::string>::const_iterator cstFind = m_mapParam.find("my_opt_check_back");
			if (cstFind == m_mapParam.end())
			{
				return S_OK;
			}

			// ״ֵ̬#userid#stoken#kek#edek#
			string strRead = cstFind->second;

			// ��ȡ�ָ��������ַ���
			std::vector<string> vecRead;
			PublicInterface::SeparateStringBystr(vecRead, strRead, MY_PARAM_END_TAG);

			if (vecRead.size() < 5)
				return S_OK;

			char chOldEDEK[USER_DB_PWD_LEN + 1] = {0};
			int nBack = 0;
			FormatDecVal(vecRead[4].c_str(), chOldEDEK, nBack);

			char chOldKEK[USER_DB_PWD_LEN + 1] = {0};
			FormatDecVal(vecRead[3].c_str(), chOldKEK, nBack);

			char chDEK[USER_DB_PWD_LEN + 1] = {0};
			// ��EDEK���н��� ���ܳ�DEK
			int nLen = UserDataASE256D((unsigned char*)chOldEDEK, USER_DB_PWD_LEN, (unsigned char*)chOldKEK, (unsigned char*)chDEK);

			char chNewKEK[USER_DB_PWD_LEN + 1] = {0};
			// �����µ�KEY
			UserDataASH256((unsigned char*)strParam.c_str(), strParam.length(), (unsigned char*)chNewKEK);

			char chNewEDEK[USER_DB_PWD_LEN + 1] = {0};
			// ���µ�KEK����DEK�����µ�EDEK
			nLen = UserDataASE256E((unsigned char*)chDEK, USER_DB_PWD_LEN, (unsigned char *)chNewKEK, (unsigned char*)chNewEDEK);

			list<string> listParam;
			// userid
			listParam.push_back(vecRead[1]);

			// kek
			string strTp;
			FormatHEXString(chNewKEK, USER_DB_PWD_LEN, strTp);
			listParam.push_back(strTp);

			// edek
			FormatHEXString(chNewEDEK, USER_DB_PWD_LEN, strTp);
			listParam.push_back(strTp);

			// stoken
			listParam.push_back(vecRead[2]);
			
			int nBackVal = CommunicateWithServer(kPInitPassword, listParam, strBack);
			if (ERR_SUCCESS != nBackVal)
			{
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"��������ʱ�������ͨ��ʧ��:%d",nBackVal));
			}

			// ��������óɹ�
			if (strBack.find(INIT_PASSWORD_SUCC) != string::npos)
			{
				string strSQL = "update  datUserInfo set edek = '";
				strSQL += strTp;
				strSQL += "' where userid = '";
				strSQL += vecRead[1];
				strSQL += "'";

				// ����EDEK�����ݿ�
				CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");
			}

		}
		
		if (pVarResult != NULL)
		{
			// ���ز���ֵ
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strBack.c_str()));
		}
		return S_OK;
	}

	else if (dispIdMember == DISPID_READ_ETK && (wFlags & DISPATCH_METHOD))
	{
		string strStoken = CBankData::GetInstance()->m_CurUserInfo.strstoken;
		if (pVarResult != NULL && strStoken.length() > 0)
		{
			string strETK;	
			string strHWID = GenHWID2();
			string strSN = CSNManager::GetInstance()->GetSN();

			// ��stoken���м���
			char chTek[33] = {0};
			string strTekMake = strSN + strHWID;
			UserDataASH256((unsigned char*)strTekMake.c_str(), strTekMake.length(), (unsigned char*)chTek);

			char* pEtk = new char[strStoken.length() + 20];
			memset(pEtk, 0, strStoken.length() + 20);

			int nLen = UserDataASE256E((unsigned char *)strStoken.c_str(), strStoken.length(), (unsigned char *)chTek, (unsigned char*)pEtk);
			FormatHEXString(pEtk, nLen, strETK);
			
			delete []pEtk;

			// ���ز���ֵ
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strETK.c_str()));
		}
		return S_OK;
	}

	else if (dispIdMember == DISPID_DEL_USERDB_BY_ID && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			std::string strParam = OLE2A(pDispParams->rgvarg[0].bstrVal);
			bool bCurUser = (strParam == CBankData::GetInstance()->m_CurUserInfo.struserid ? true:false);

			// ����ǵ�ǰ��¼���û����˳���ǰ��¼
			if (bCurUser)
				::SendMessage(g_hMainFrame, WM_USER_INFO_MENU_CLICKED, MY_USER_INFO_MENU_CLICK_QUIT, 0);

			// ɾ�����ݿ�
			CBankData::GetInstance()->DeleteUserDbByID(strParam.c_str(), true);

		}
		return S_OK;
	}

	else if (dispIdMember == DISPID_CHANGE_WIN_NAME && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			std::string strParam = OLE2A(pDispParams->rgvarg[0].bstrVal);
			::SendMessage(g_hMainFrame, WM_SETTEXT, WM_CHANGE_USER_DLG_NAME, (LPARAM)strParam.c_str());

		}
		return S_OK;
	}

	// �������������
	else if (dispIdMember == DISPID_CHANGE_MIAL_PWD && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR && pDispParams->rgvarg[1].vt == VT_BSTR && pDispParams->rgvarg[2].vt == VT_BSTR &&
			pDispParams->rgvarg[3].vt == VT_BSTR)
		{
			std::string strType = OLE2A(pDispParams->rgvarg[3].bstrVal); // mial��password
			std::string strUserID = OLE2A(pDispParams->rgvarg[2].bstrVal); // �û�ID
			std::string strValue = OLE2A(pDispParams->rgvarg[1].bstrVal); // ֵ
			std::string strOldPwd = OLE2A(pDispParams->rgvarg[0].bstrVal); // ԭʼ���룬У��ʱʹ��

			char pOldKek[33] = {0};
			// ����KEY
			UserDataASH256((unsigned char*)strOldPwd.c_str(), strOldPwd.length(), (unsigned char*)pOldKek);


			string strHWID = GenHWID2();
			wstring wstrHWID = CA2W(strHWID.c_str());
			string strSN = CSNManager::GetInstance()->GetSN();

			string strSub,strTp;
			std::string strParam = "xml=";
			strParam += strUserID;
			if (strType == "password")
			{
				strParam += MY_PARAM_END_TAG;
				
				// ����KEY
				char pNewKek[33] = {0};
				UserDataASH256((unsigned char*)strValue.c_str(), strValue.length(), (unsigned char*)pNewKek);

				// ���NEWKEK
				FormatHEXString(pNewKek, 32, strTp);
				strParam += strTp;
				strParam += MY_PARAM_END_TAG;

				// �������DEK
				//unsigned char chValue[USER_DB_PWD_LEN + 1] = {0};
				//Rand20ByteUCharData(chValue, USER_DB_PWD_LEN);

				char chOldEDEK[USER_DB_PWD_LEN + 1] = {0};
				char chDEK[USER_DB_PWD_LEN + 1] = {0};
				char pNewEDEK[USER_DB_PWD_LEN + 1] = {0};

				// ��ȡOldEDEK
				string  strSQL = "select edek from datUserInfo where userid = '";
				strSQL += strUserID;
				strSQL += "'";
				string strQueryVal = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");
				string strFind = "edek\":\"";
				if (strQueryVal.find(strFind) != string::npos)
				{
					strQueryVal = strQueryVal.substr(strQueryVal.find(strFind) + strFind.length(), strQueryVal.length());
					strQueryVal = strQueryVal.substr(0, strQueryVal.find("\""));

					// ��ʮ�����Ʒ������ʮ��������
					int nBack = 0;
					FormatDecVal(strQueryVal.c_str(), chOldEDEK, nBack);

					// ��EDEK���н��� ���ܳ�DEK
					UserDataASE256D((unsigned char*)chOldEDEK, USER_DB_PWD_LEN, (unsigned char*)pOldKek, (unsigned char*)chDEK);

					// ��KEK��������EDEK
					UserDataASE256E((unsigned char*)chDEK, USER_DB_PWD_LEN, (unsigned char *)pNewKek, (unsigned char*)pNewEDEK);
				}


				// ���EDEK
				string strNewEDEK;
				FormatHEXString(pNewEDEK, USER_DB_PWD_LEN, strNewEDEK);
				strParam += strNewEDEK;
				strParam += MY_PARAM_END_TAG;

				// ���OldKek
				FormatHEXString(pOldKek, USER_DB_PWD_LEN, strTp);
				strParam += strTp;
				strParam += MY_PARAM_END_TAG;

				// ���SN
				//strParam += strSN;
				//strParam += MY_PARAM_END_TAG;

				//// ���HID
				//strParam += strHWID;
				//strParam += MY_PARAM_END_TAG;

				
				// ���MAIL
				strParam += CBankData::GetInstance()->m_CurUserInfo.strmail;
				strParam += MY_PARAM_END_TAG;

				
				// ���stoken
				strParam += CBankData::GetInstance()->m_CurUserInfo.strstoken;
				strParam += MY_PARAM_END_TAG;

				CDownloadThread downloadThread;
				downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPChangePassword)).c_str(), (LPSTR)strParam.c_str());
				char chTemp[1024] = {0};
				DWORD dwRead = 0;
				int nBackVal = downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead);
				if (ERR_SUCCESS != nBackVal)
				{
					CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"�޸�����ʱ�������ͨ��ʧ��:%d",nBackVal));
				}
				else
				{
					string strRead = chTemp;
					if (strRead.find(MY_PARAM_END_TAG) != string::npos)
						strSub = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

					if (strSub.find(PWD_CHANGE_SUCC) != string::npos) // ���Ϊ71���ɹ�
					{
						// ��ȡstoken
						string strTag = MY_PARAM_END_TAG;
						strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
						string strStoken = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

						// ���µ�ǰ�û���stoken
						CBankData::GetInstance()->m_CurUserInfo.strstoken = strStoken;
						// ����UI�е�ǰ�û���stoken
						::SendMessage(g_hMainFrame, WM_SETTEXT, WM_NOTIFYUI_CUR_USER_STOKEN, (LPARAM)strStoken.c_str());

						//strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
						//string strEDEK = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

						

							// ���鵱ǰ�û��Ƿ����Զ���¼�û�������ǣ������stoken�����򲻸���
							string  strSQL = "select userid from datUserInfo where userid = '";
							strSQL += strUserID;
							strSQL += "' and autoload = 1";
							string strAutoLoad = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");

							// ����EDEK�� stoken
							strSQL = "update  datUserInfo set edek = '";
							strSQL += strNewEDEK;
							if (strAutoLoad.find(strUserID) != string::npos)
							{

								// ��stoken���м���
								char chTek[33] = {0};
								string strTekMake = strSN + strHWID;
								UserDataASH256((unsigned char*)strTekMake.c_str(), strTekMake.length(), (unsigned char*)chTek);

								char* pEtk = new char[strStoken.length() + 20];
								memset(pEtk, 0, strStoken.length() + 20);

								int nLen = UserDataASE256E((unsigned char *)strStoken.c_str(), strStoken.length(), (unsigned char *)chTek, (unsigned char*)pEtk);
								FormatHEXString(pEtk, nLen, strTp);
								strSQL += "', etk = '";
								strSQL += strTp;
								delete []pEtk;
							}
							strSQL += "'";
							strSQL += " where userid = '";
							strSQL += strUserID;
							strSQL += "'";

							// ��¼stoken,EDEK��DataDB.dat��
							CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");

							//char chOldEDEK[33] = {0};
							//char chOldPwd[USER_DB_PWD_LEN + 1] = {0};

							// ��ʮ�����Ʒ������ʮ��������
							//int nBack = 0;
							//FormatDecVal(strQueryVal.c_str(), chOldEDEK, nBack);

							// ��EDEK���н���
							//UserDataASE256D((unsigned char*)chOldEDEK, 32, (unsigned char*)pOldKek, (unsigned char*)chOldPwd);

							//string strDBPath = strUserID + ".dat";
							// �޸��û�����
							//CBankData::GetInstance()->ChangeUserDBPwd((LPSTR)strDBPath.c_str(), (char*)chOldPwd, USER_DB_PWD_LEN, (char*)chValue, USER_DB_PWD_LEN);
							// ʹ��������򿪿�
							//CBankData::GetInstance()->SetCurrentUserDB((LPSTR)strDBPath.c_str(), (char*)chValue, USER_DB_PWD_LEN);


							// ֪ͨUI���µ�ǰ�û���
							//string strSendUI = strDBPath;
							//strSendUI += MY_PARAM_END_TAG;
							//string strPWD;
							//FormatHEXString((char*)chValue, USER_DB_PWD_LEN, strPWD);
							//strSendUI += strPWD;
							//strSendUI += MY_PARAM_END_TAG;
							//::SendMessage(g_hMainFrame, WM_SETTEXT, WM_NOTIFYUI_UPDATE_USER_DB, (LPARAM)strSendUI.c_str());
						

					}
					else // У��stoken�ı༭ʱ��,�����Ƿ�Ҫ�˳���ǰ��¼���ط��ÿ����
					{
						if (NeedExitCurrentLoad(chTemp) || (strSub.find(PWD_CHANGE_MAIL_ERR) != string::npos) || (strSub.find(PWD_CHANGE_STOKEN_ERR) != string::npos))
						{
							// �˳���ǰ��¼
							::SendMessage(g_hMainFrame, WM_USER_INFO_MENU_CLICKED, MY_USER_INFO_MENU_CLICK_QUIT, 0);
							//strSub = "    ";
							//strSub += MANU_LOAD_PWD_OUTOFTIME; // �����ѹ���
						}
						
					}
				}

			}
			else if (strType == "mail")// �޸�����
			{
				// ����uID, mail
				strParam += MY_PARAM_END_TAG;

				// ���mail
				strParam += strValue;
				strParam += MY_PARAM_END_TAG;

				
				// ���OldKek
				FormatHEXString(pOldKek, 32, strTp);
				strParam += strTp;
				strParam += MY_PARAM_END_TAG;

				
				// ���SN
				//strParam += strSN;
				//strParam += MY_PARAM_END_TAG;

				//// ���HID
				//strParam += strHWID;
				//strParam += MY_PARAM_END_TAG;

				// ԭʼ����
				strParam += CBankData::GetInstance()->m_CurUserInfo.strmail;
				strParam += MY_PARAM_END_TAG;

				CDownloadThread downloadThread;
				downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPChangeMail)).c_str(), (LPSTR)strParam.c_str());
				char chTemp[1024] = {0};
				DWORD dwRead = 0;
				int nBackVal = downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead);
				if (ERR_SUCCESS != nBackVal)
				{
					CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, CRecordProgram::GetInstance()->GetRecordInfo(L"�޸�����ʱ�������ͨ��ʧ��:%d",nBackVal));
				}
				else
				{
					//if () ���Ϊ41���ɹ������43���Ѿ�����
					string strRead = chTemp;
					if (strRead.find(MY_PARAM_END_TAG) != string::npos)
						strSub = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

					if (strSub.find(MAIL_CHANGE_SUCC) != string::npos) // ���Ϊ72���ɹ�
					{
						// ��ȡmail
						string strTag = MY_PARAM_END_TAG;
						strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
						string strNewStoken = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));


						// ���鵱ǰ�û��Ƿ����Զ���¼�û�������ǣ������stoken�����򲻸���
						string  strSQL = "select userid from datUserInfo where userid = '";
						strSQL += strUserID;
						strSQL += "' and autoload = 1";
						string strQueryVal = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");

						// ����mail�� stoken
						strSQL = "update  datUserInfo set mail = '";
						strSQL += strValue;
						if (strSQL.find(strUserID) != string::npos)
						{
							// ��stoken���м���
							char chTek[33] = {0};
							string strTekMake = strSN + strHWID;
							UserDataASH256((unsigned char*)strTekMake.c_str(), strTekMake.length(), (unsigned char*)chTek);

							char* pEtk = new char[strNewStoken.length() + 20];
							memset(pEtk, 0, strNewStoken.length() + 20);

							int nLen = UserDataASE256E((unsigned char *)strNewStoken.c_str(), strNewStoken.length(), (unsigned char *)chTek, (unsigned char*)pEtk);
							FormatHEXString(pEtk, nLen, strTp);
							strSQL += "', etk = '";
							strSQL += strTp;
							delete []pEtk;
						}
						strSQL += "' where userid = '";
						strSQL += strUserID;
						strSQL += "'";

						// ��¼mail,stoken��DataDB.dat��
						CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");
						
						CAxUI::UpdateUserInfo(strNewStoken.c_str(), strValue.c_str(), strUserID.c_str(), (int)emUserNet, true, MY_MAIL_VERIFY_ERROR);
					}
					else
					{
						if (NeedExitCurrentLoad(chTemp) || strSub.find(CURMAIL_CAN_NOT_FIND) != string::npos) // ԭʼ���䲻����
						{
							// �˳���ǰ��¼
							::SendMessage(g_hMainFrame, WM_USER_INFO_MENU_CLICKED, MY_USER_INFO_MENU_CLICK_QUIT, 0);
							//strSub = "    ";
							//strSub += MANU_LOAD_PWD_OUTOFTIME; // �����ѹ���
						}
					}
				}
			}

			if (pVarResult != NULL)
			{
				// ���ز���ֵ
				pVarResult->vt = VT_BSTR;
				pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strSub.c_str()));
			}
			return S_OK;
		}
	}
	else if (dispIdMember == DISPID_GET_CUR_USERID && (wFlags & DISPATCH_METHOD))
	{
		// δ��¼ʱ����"Guest"
		if (pVarResult != NULL)
		{
			string strUserID = CBankData::GetInstance()->m_CurUserInfo.struserid;
			// ���ز���ֵ
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)A2COLE(strUserID.c_str()));
		}
		return S_OK;
	}

	else if (dispIdMember == DISPID_SET_GUIDE_INFO && (wFlags & DISPATCH_METHOD))
	{
		if(pDispParams->rgvarg[0].vt == VT_BSTR)
		{
			string strTp = OLE2A(pDispParams->rgvarg[0].bstrVal);
			// д��ע���
			if (ERROR_SUCCESS == ::SHSetValueA(HKEY_CURRENT_USER, REGEDIT_MONHUB_PATH,MONHUB_GUIDEINFO_KEY, REG_SZ, strTp.c_str(), strTp.length()))
			{
			}
		}
		return S_OK;
	}
	
	else if (dispIdMember == DISPID_GET_CUR_STATUS && (wFlags & DISPATCH_METHOD))
	{
		CHANGESTATUS emParam = CBankData::GetInstance()->m_CurUserInfo.emChangeStatus;
		if (pVarResult != NULL)
		{
			if (emUserNoChange == emParam)
			{
				// �û��ѵ�¼
				if (MONHUB_GUEST_USERID != CBankData::GetInstance()->m_CurUserInfo.struserid)
				{
					emParam = emUserLoad;
				}
			}

			// ���ز���ֵ
			pVarResult->vt = VT_BSTR;
			CString strTemp;
			strTemp.Format(L"%d", emParam);
			pVarResult->bstrVal = ::SysAllocString((LPOLESTR)(LPCTSTR)strTemp);
		}
		return S_OK;
	}

	
	else if (dispIdMember == DISPID_QUERY_USER_LOAD && (wFlags & DISPATCH_METHOD))
	{
		// �����˳��ӿ�
		::SendMessage(g_hMainFrame, WM_USER_INFO_MENU_CLICKED, MY_USER_INFO_MENU_CLICK_QUIT, 0);
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

	else if (dispIdMember == DISPID_GET_BILL_LIST && (wFlags & DISPATCH_METHOD))
	{
		// ��ȡ�˵�	
		INT64 id = 0;
		if(pDispParams->rgvarg[3].vt == VT_BSTR)
		{
			if(pDispParams->rgvarg[3].bstrVal != NULL)
				id = _atoi64(OLE2A(pDispParams->rgvarg[3].bstrVal));// �˻�id
		}
		else if(pDispParams->rgvarg[3].vt == VT_I8)
			id = pDispParams->rgvarg[3].llVal;

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

// ���б��ص�¼
int CExternalDispatchImpl::ExcuteLocalLoad(const char* pMail, const unsigned char* pKek, int nLen, string& strUserID)
{
	strUserID.clear();

	ATLASSERT(NULL != pMail && NULL != pKek);
	if (NULL == pMail || NULL == pKek)
		return atoi(MANU_LOAD_LAN_ERR);

	// ��ȡOldEDEK,���ܳ����ݿ�����
	string strSQL = "select userid, edek from datUserInfo where mail = '";
	strSQL += pMail;
	strSQL += "'";
	string strQueryVal = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");
	string strFind = "userid\":\"";
	if (strQueryVal.find(strFind) == string::npos)
		return atoi(MAUN_LOAD_LAN_FIRST);

	strQueryVal = strQueryVal.substr(strQueryVal.find(strFind) + strFind.length(), strQueryVal.length());
	strUserID = strQueryVal.substr(0, strQueryVal.find("\""));

	if (strUserID.length() <= 0)
		return atoi(MAUN_LOAD_LAN_FIRST);

	strFind = "edek\":\"";
	if (strQueryVal.find(strFind) == string::npos)
		return atoi(MAUN_LOAD_LAN_FIRST);

	strQueryVal = strQueryVal.substr(strQueryVal.find(strFind) + strFind.length(), strQueryVal.length());
	strQueryVal = strQueryVal.substr(0, strQueryVal.find("\""));

	char chOldEDEK[33] = {0};
	char chOldPwd[USER_DB_PWD_LEN + 1] = {0};

	// ��ʮ�����Ʒ������ʮ��������
	int nBack = 0;
	FormatDecVal(strQueryVal.c_str(), chOldEDEK, nBack);

	// ��EDEK���н���
	UserDataASE256D((unsigned char*)chOldEDEK, 32, (unsigned char*)pKek, (unsigned char*)chOldPwd);

	string strDataPath = strUserID + ".dat";
	// �޸ĵ�ǰ�û�����
	bool bVal = CBankData::GetInstance()->SetCurrentUserDB((LPSTR)strDataPath.c_str(), chOldPwd, USER_DB_PWD_LEN);


	// ֪ͨUI���µ�ǰ�û���
	string strSendUI = strDataPath;
	strSendUI += MY_PARAM_END_TAG;
	string strPWD;
	FormatHEXString(chOldPwd, 32, strPWD);
	strSendUI += strPWD;
	strSendUI += MY_PARAM_END_TAG;
	::SendMessage(g_hMainFrame, WM_SETTEXT, WM_NOTIFYUI_UPDATE_USER_DB, (LPARAM)strSendUI.c_str());
	/*string strSendUI = strDataPath;
	strSendUI += MY_PARAM_END_TAG;
	strSendUI += chOldPwd;
	strSendUI += MY_PARAM_END_TAG;
	::SendMessage(g_hMainFrame, WM_SETTEXT, WM_NOTIFYUI_UPDATE_USER_DB, (LPARAM)strSendUI.c_str());*/

	if (bVal)
		return atoi(MANU_LOAD_LAN_SUCC); // �ֶ���¼�ɹ�
	else
		return atoi(MANU_LOAD_LAN_ERR);// �ֶ���¼ʧ��

}

// �������һ��20�ֽڳ����޷�����
void CExternalDispatchImpl::Rand20ByteUCharData(unsigned char* pStore, int nLen)
{
	ATLASSERT(NULL != pStore && USER_DB_PWD_LEN == nLen);
	if (NULL == pStore || USER_DB_PWD_LEN != nLen)
		return;

	// �������DEK
	srand((unsigned)time(NULL));
	int scope = 255;
	int i = 0;
	//unsigned char chValue[USER_DB_PWD_LEN + 1] = {0};
	while( i < USER_DB_PWD_LEN)
	{
		pStore[i] = rand() % scope + 1;// ���������
		i ++;
	}
}

// Base64Encode
//bool CExternalDispatchImpl::Base64Encode(char *pData, int nLen, string& strEncode)
//{
//	ATLASSERT (NULL != pData && nLen > 0);
//	if (NULL == pData || nLen <= 0)
//		return false;
//	strEncode.clear();
//
//	int dwSize = nLen * 2 + 1;
//	unsigned char* pszOut = new unsigned char[dwSize];
//	base64_encode((LPBYTE)pData, nLen, pszOut, &dwSize);
//	pszOut[dwSize] = 0;
//
//	
//	strEncode = (char*)pszOut;
//
//	delete []pszOut;
//
//	return true;
//}

bool CExternalDispatchImpl::NeedExitCurrentLoad(const char* pServerRead)
{
	ATLASSERT(NULL != pServerRead);
	if (NULL == pServerRead)
		return false;

	string strRead = pServerRead;
	string strTag = MY_PARAM_END_TAG;
	int nIndex = strRead.find(MY_PARAM_END_TAG);
	if (string::npos == nIndex)
		return false;

	string strStatus = strRead.substr(0, nIndex);
	// ֻ�е�¼Ϊʧ��
	if (strStatus.find(MANU_LOAD_PWD_ERR) == string::npos)
		return false;

	strRead = strRead.substr(nIndex + strTag.length(), strRead.length());

	nIndex = strRead.find(MY_PARAM_END_TAG);
	// �ӷ������õ����µ�stoken��ʱ��
	if (string::npos == nIndex)
		return false;

	strRead = strRead.substr(0, nIndex);

#define STOKEN_TIME_LEN 12 // stoken��ʱ��ĳ���
#define STOKEN_TIME_BEGIN 8 // stoken��ʱ�����ʼλ��

	string strStoken = CBankData::GetInstance()->m_CurUserInfo.strstoken;
	// ȡstokenʱ��
	string strSTime = strStoken.substr(STOKEN_TIME_BEGIN, strStoken.length());
	strSTime = strSTime.substr(0, STOKEN_TIME_LEN);

	__int64 i64Sever = _atoi64(strRead.c_str());
	__int64 i64Stoken = _atoi64(strSTime.c_str());

	// �����¼ʱstoken�е�ʱ��͸մӷ�������ȡ����ʱ�䲻һ�£��û��������������޸Ĺ����룬�˳���ǰ��½
	if (i64Sever > i64Stoken)
	{
		return true;
	}

	return false;
}

void CExternalDispatchImpl::AddJSParam(const char* pPName, const char* pPVal)
{
	ATLASSERT(NULL != pPName && NULL != pPVal);
	if (NULL == pPName || NULL == pPVal)
		return;

	std::map<std::string, std::string>::const_iterator cstIt;
	cstIt = m_mapParam.find(pPName); // �����Ƿ��Ѿ�����
	if (cstIt != m_mapParam.end())
	{
		// �Ѿ�����
		m_mapParam.erase(cstIt);
	}

	m_mapParam.insert(std::make_pair(pPName, pPVal));
}

// ���ĵ�ǰ�û�ͬ����صı���
void CExternalDispatchImpl::ChangeCurUserSynchroStatus()
{
	// �ı�ͬ����ť��ɫ
	::SendMessage(g_hMainFrame, WM_NOTIFYUI_SYNCHRO_BTN_CHANGE, 0, 0);

	// ����DataDB���needsynchro�ֶ�(���ڴ˲��ֹ��ܿ���ȥ��)
	string strSQL = "update datUserInfo set needsynchro = 1 where userid = '";
	strSQL += CBankData::GetInstance()->m_CurUserInfo.struserid;
	strSQL += "'";
	CBankData::GetInstance()->ExecuteSQL(strSQL, "DataDB");

	m_sbUpdateSynchroBtn = true;
}

// �����������ͨѶ
int CExternalDispatchImpl::CommunicateWithServer(int nEmSite, list<string> listParam, string& strBack)
{
	webconfig emSite = (webconfig)nEmSite;
	ATLASSERT( emSite > kBeginTag && emSite < kEndTag);
	if (emSite <= kBeginTag || emSite >= kEndTag)
		return false;

	string strSend;
	if (listParam.size() > 0)
	{
		strSend = "xml=";
	}

	// �ϲ�����
	list<string>::const_iterator cstIt = listParam.begin();
	for (; cstIt != listParam.end(); cstIt ++)
	{
		strSend += (*cstIt);
		strSend += MY_PARAM_END_TAG;
	}

	char chTemp[1024] = {0};
	wstring wstrHWID = CA2W(GenHWID2().c_str());

	CDownloadThread downloadThread;
	downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(emSite)).c_str(), (LPSTR)strSend.c_str());
	DWORD dwRead = 0;
	int nBackVal = downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead);
	
	strBack = chTemp;

	return nBackVal;

}