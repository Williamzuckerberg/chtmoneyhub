/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  AxUI.cpp
 *      ˵����  �ؼ�����
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.22	���ź�ͨ	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#include "stdafx.h"
#include "AxUI.h"
#include "../Security/BankProtector/export.h"
#include "../BankUI/Util/CleanHistory.h"
#include "ExternalDispatchImpl.h"
#include "..\BankData\BankData.h"
#include "../Utils/UserBehavior/UserBehavior.h"
#include "BankData/USBMonitor.h"
#include "../Utils/ListManager/ListManager.h"
#include "../Utils/HardwareID/genhwid.h" // ��ȡӲ��ID
#include "../Utils/SN/SNManager.h" // ��ȡSN
#include "../Utils/SecurityCache/comm.h"
#include <vector>
#include <algorithm>
using namespace std;

extern HWND g_hMainFrame;
#define  MH_SENDSTARTDELAYEVENT (0xfe02)

CAxUI::CAxUI() :m_hTag(NULL), m_pbilldata(NULL)
{
	Create(HWND_MESSAGE); // call WM_CREATE
}


//////////////////////////////////////////////////////////////////////////

int CAxUI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CExternalDispatchImpl::m_hAxui = m_hWnd;
	DWORD dwThreadID;
	CloseHandle(CreateThread(NULL, 0, _threadInit, this, 0, &dwThreadID));
	return 0;
}

//////////////////////////////////////////////////////////////////////////

DWORD WINAPI ThreadProcCreateAxControl(LPVOID lpParameter);

DWORD WINAPI CAxUI::_threadInit(LPVOID lp)
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"AxUI��ʼ��ʼ��");

	CAxUI* pThis = (CAxUI*)lp;
	if(lp == NULL)
		return 0;

	while (pThis->m_hTag == NULL)
	{
		pThis->m_hTag = CExternalDispatchImpl::m_hFrame[0];
		if (NULL != pThis->m_hTag)
		{

			// �ȳ�ʼ��USB���
			CUSBMonitor::GetInstance()->InitParam();
			pThis->CheckDownloadFile();

			//����usb����߳�
			CUSBMonitor::GetInstance()->StartUSBMonitor();
			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"AxUI������USB����߳�");

			break;
		}
		Sleep(200);
	}
	return 0;
}

LRESULT CAxUI::OnCreateNewPage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"AxUI ��������ҳ");
	DWORD dwThreadID;
	HANDLE hThread = ::CreateThread(NULL, 0, ThreadProcCreateAxControl, (LPVOID)lParam, NULL, &dwThreadID);
	::CloseHandle(hThread);
	return 0;
}

LRESULT CAxUI::OnExit(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CleanHistory();
	//CUserBehavior::GetInstance ()->Action_SendDataToServerWhenExit (); // �������ݵ���������//��ֹ�����������������˳�����

	::TerminateProcess(::GetCurrentProcess(), 0);

	return 0;
}

void CAxUI::CheckDownloadFile(void)
{
	CBkInfoDownloadManager *pTemp = CBkInfoDownloadManager::GetInstance ();
	pTemp->SetProgressNotifyHwnd (CExternalDispatchImpl::m_hFrame[0]);
	if (NULL != pTemp)
		pTemp->FinishDLBreakFile (); // ���������û�����δ��ɵ����пؼ�
}
//ȡ���ղ�
LRESULT CAxUI::OnCancelAddFav(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(CExternalDispatchImpl::m_hFrame[0])
		::PostMessageW(CExternalDispatchImpl::m_hFrame[0], WM_AX_FRAME_DELETEFAV, 0, lParam);

	LONG tlfid = lParam;
	char tfid[6] = {0};

	memcpy(tfid, (void*)&tlfid, 4);
	USES_CONVERSION;
	std::wstring appId(A2W(tfid));
	vector<wstring>::iterator ite = find( uacPopVec.begin() , uacPopVec.end() , appId) ;
	if(ite != uacPopVec.end())
		uacPopVec.erase(ite);
	return 0;
}

LRESULT CAxUI::UserAutoLoad(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	string strUserID, strMail, strStoken;
	CBankData::GetInstance()->ReadNeedAutoLoadUser(strUserID, strMail, strStoken);

	string strParam = "xml=" + strUserID + MY_PARAM_END_TAG;


	string strHWID = GenHWID2();
	wstring wstrHWID = CA2W(strHWID.c_str());
	string strSN = CSNManager::GetInstance()->GetSN();

	strParam += strSN;
	strParam += MY_PARAM_END_TAG;

	strParam += strHWID;
	strParam += MY_PARAM_END_TAG;

	
	strParam += strStoken;
	strParam += MY_PARAM_END_TAG;

	CDownloadThread downloadThread;
	downloadThread.DownLoadInit(wstrHWID.c_str(), L"http://moneyhub.ft.com/server/auto_log_on.php", (LPSTR)strParam.c_str());
	char chTemp[1024] = {0};
	DWORD dwRead = 0;
	if (ERR_SUCCESS != downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead))
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"�Զ���½ʧ��");

		// ��¼ʧ�ܣ�����һ���յ��˺�
		::SendMessage(g_hMainFrame, WM_SETTEXT, WM_FINIHS_AUTO_LOAD, (LPARAM)L"");

		// ���õ�¼ʧ�ܵ�״̬
		return 0;
	}

	// ȡ�����ؽ����KEK��EDEK
	string strRead = chTemp;
	string strSub = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

#define MY_AUTO_LAOD_SUCC			 "61"
	int nParam = -1;
	if (strSub.find(MY_AUTO_LAOD_SUCC) != string::npos)
	{
		// ��ȡKEK
		string strTag = MY_PARAM_END_TAG;
		strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
		string strKEK = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

		// ��ȡEDEK
		strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
		string strEDEK = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

		// ��KEK EDEKlת����ʵ��ֵ
		char kekVal[33] = {0};
		char edekVal[33] = {0};
		char keyVal[33] = {0};
		int nBack = 0;
		FormatDecVal(strKEK.c_str(), kekVal, nBack);
		FormatDecVal(strEDEK.c_str(), edekVal, nBack);

		UserEdekUnPack((unsigned char*)edekVal, 32, (unsigned char*)kekVal, (unsigned char*)keyVal);

		// ʹ��USERID�������ݿ�
		string strData = strUserID;
		strData += ".dat";

		// �����û��Ŀ����óɵ�ǰ���ݿ�,���������
		CBankData::GetInstance()->SetCurrentUserDB((LPSTR)strData.c_str(), keyVal, 32);

		// ���ĵ�ǰ�û���Ϣ
		CBankData::GetInstance()->m_CurUserInfo.strstoken = strStoken;
		CBankData::GetInstance()->m_CurUserInfo.strmail = strMail;
		CBankData::GetInstance()->m_CurUserInfo.struserid = strUserID;
	}
	else
	{
		// ���MAIL��������Ϣʱ��ʾ��¼ʧ��
		strMail.clear();
	}


	// ֪ͨUI���µ�ǰ�û���Ϣ��ͬ����UI��Ҫʹ����Щֵ��
	string strMesParam = strStoken + MY_PARAM_END_TAG;
	strMesParam += strMail;
	strMesParam += MY_PARAM_END_TAG;
	strMesParam += strUserID;
	strMesParam += MY_PARAM_END_TAG;

	::SendMessage(g_hMainFrame, WM_SETTEXT, WM_UPDATE_USER_STATUS, (LPARAM)strMesParam.c_str());

	/*CString cstrMail = CA2W(strMail.c_str());
	::SendMessage(g_hMainFrame, WM_SETTEXT, WM_FINIHS_AUTO_LOAD, (LPARAM)(LPCTSTR)cstrMail);*/
	return 0;
}

LRESULT CAxUI::OnLoadUserQuit(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	string strPw;
	int nLen = 0;

// �����汾���������������
#ifdef OFFICIAL_VERSION
	strPw = "NCrFT2RIeD0NY2wHOI8W";
	nLen = 20;
#endif
	// �����ݿ�ת�����ÿ����ݿ�
	CBankData::GetInstance()->SetCurrentUserDB("Guest.dat", (LPSTR)strPw.c_str(), nLen);

	// ���鵱ǰ�û��Ƿ����Զ���¼�û�������ǣ������stoken�����򲻸���
	string strUserID = CBankData::GetInstance()->m_CurUserInfo.struserid;	
	string strSQL = "select userid from datUserInfo where userid = '";
	strSQL += strUserID;
	strSQL += "' and autoload = 1";
	string strQueryVal = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");
	if (strQueryVal.find(strUserID) != string::npos)
	{
		strSQL = "update datUserInfo set autoload = 0, stoken = '' where userid = '";
		strSQL += strUserID;
		strSQL += "'";
		CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");
	}

	// ���ĵ�ǰ�û���Ϣ
	CBankData::GetInstance()->m_CurUserInfo.strstoken.clear();
	CBankData::GetInstance()->m_CurUserInfo.strmail.clear();
	CBankData::GetInstance()->m_CurUserInfo.struserid = "Guest";



	// ֪ͨUI���µ�ǰ�û���Ϣ��ͬ����UI��Ҫʹ����Щֵ��
	string strMesParam = "Guest";
	strMesParam += MY_PARAM_END_TAG;
	strMesParam += MY_PARAM_END_TAG;
	strMesParam += MY_PARAM_END_TAG;
	//::PostMessage(g_hMainFrame, WM_SETTEXT, WM_NOTICE_UI_UPDATE_USERINFO, (LPARAM)strMesParam.c_str());

	::SendMessage(g_hMainFrame, WM_SETTEXT, WM_UPDATE_USER_STATUS, (LPARAM)L"");
	return 0;
}

//LRESULT CAxUI::OnShowUserDlg(UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	if(CExternalDispatchImpl::m_hFrame[0])
//		::PostMessageW(CExternalDispatchImpl::m_hFrame[0], WM_AX_SHOW_USER_DLG, 0, lParam);
//	OutputDebugStringA("send message ksksk");
//	return 0;
//}

//LRESULT CAxUI::OnDownLoadBankInfo (UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	std::string strTp = (LPSTR)lParam;
//
//	if (NULL == m_pBkDownloadManager)
//	{
//		HWND hwnd = (HWND)wParam;
//		CString cstrT;
//		cstrT.Format (L"accept hwnd = %d", wParam);
//		
//		m_pBkDownloadManager = new CBkInfoDownloadManager();
//	}
//
//	// �����Ƿ��Ѿ���װ
//	if (m_pBkDownloadManager->IsSetupAlready ((char*)strTp.c_str ()))
//		return S_FALSE; // �����װ�˾Ͳ�����
//
//	// ִ������
//	m_pBkDownloadManager->DownloadBankCtrl ((char*)strTp.c_str ());
//	return S_OK;
//}


//LRESULT CAxUI::OnUSBChange(UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	// ת������
//	DWORD mid = (DWORD)wParam;
//	LONG fid = (LONG)lParam;
//	int nVid = fid & 0xffffffff; // ���ֽ�
//	int nPid = fid >> 32; // ���ֽ�
//
//	CBankData* pBankData = CBankData::GetInstance();
//
//	// ��ȡ��USB�����ݿ��б���ļ�¼
//	USBRECORD usbRecord;
//	if (!pBankData->GetAUSBRecord (nVid, nPid, mid, usbRecord))
//		return S_FALSE;
//
//	if (_SETUP_FINISH == usbRecord.status)
//		return S_OK;
//
//	if (_DOWNLOAD_FINISH == usbRecord.status)// ����Ѿ����غ��ˣ����а�װ
//	{
//		STARTUPINFO si;	
//		PROCESS_INFORMATION pi;	
//		ZeroMemory( &pi, sizeof(pi) );	
//		ZeroMemory( &si, sizeof(si) );	
//		si.cb = sizeof(si);	
//		// ���а�װ
//		if(CreateProcessW(NULL, NULL/*(LPWSTR)path.c_str()*/, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) // ·����û��ȷ��
//		{		
//			CloseHandle( pi.hProcess );		
//			CloseHandle( pi.hThread );		
//		}
//	} 
//
//	if (usbRecord.status >= 0 && usbRecord.status < _DOWNLOAD_FINISH) // ���û�����ػ�����δ��ɣ���������
//	{
//		/*if (NULL == m_pBkDownloadManager)
//		{
//			if(CExternalDispatchImpl::m_hFrame[0])
//				m_pBkDownloadManager = new CBkInfoDownloadManager(CExternalDispatchImpl::m_hFrame[0]);
//		}*/
//		CBkInfoDownloadManager *pTemp = CBkInfoDownloadManager::GetInstance ();
//
//		if (NULL != pTemp)
//		{
//
//			std::string strXml = usbRecord.xml;
//
//			// ִ������
//			pTemp->DownloadBankCtrl (usbRecord.fav, NULL, NULL, (LPVOID)strXml.c_str (), strXml.size ());
//		}
//	}
//
//	return 0;
//} wParam, LPARAM lParam)

LRESULT CAxUI::OnGetAllBill(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char *pInfo = (char*)lParam;
	if(pInfo == NULL)
		return 0;

	m_pbilldata = pInfo;
	return 0;
}

LRESULT CAxUI::OnSendToBillPage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_pbilldata != NULL)
		SetTimer(MH_SENDSTARTDELAYEVENT, 1 * 1000 ,NULL);	
	return 0;
}

LRESULT CAxUI::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(wParam == MH_SENDSTARTDELAYEVENT)
	{
		KillTimer(MH_SENDSTARTDELAYEVENT);
		::PostMessageW(CExternalDispatchImpl::m_hFrame[2], WM_AX_GET_ALL_BILL, NULL, (LPARAM)&m_pbilldata);
	}
	return 0;
}


































