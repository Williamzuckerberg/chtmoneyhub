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
#include "..//Utils/Config//HostConfig.h"
#include "..\BankUI\UIControl\CoolMessageBox.h" // mhMessageBox
#include <vector>
#include <algorithm>
#include <atlcomtime.h>
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

			// ����ϵͳʱ��
			pThis->CheckSystemTime();

			//����usb����߳�
			CUSBMonitor::GetInstance()->StartUSBMonitor();
			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"AxUI������USB����߳�");

			break;
		}
		Sleep(200);
	}
	return 0;
}

void CAxUI::CheckSystemTime(void)
{
	wstring wstrHWID = CA2W(GenHWID2().c_str());
	CDownloadThread downloadThread;
	downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPUserServerTime)).c_str(), (LPSTR)"");
	char chTemp[1024] = {0};
	DWORD dwRead = 0;
	if (ERR_SUCCESS != downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead))
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"��ȡ������ʱ��ʧ��");
		return;
	}
	else
	{
		string strRead = chTemp;
		if(strRead.size() <= 0)
			return;
		// ��ȡKEK
		string strTag = MY_PARAM_END_TAG;
		strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
		string strServerTime = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));
		if (strServerTime.length() <= 0)
			return;

		COleDateTime oleTServer;
		oleTServer.ParseDateTime(CA2W(strServerTime.c_str()));
		COleDateTime oleTCurrent = COleDateTime::GetCurrentTime();
		COleDateTimeSpan oleTSpan1 = oleTServer - oleTCurrent;
		COleDateTimeSpan oleTSpan2 = oleTCurrent - oleTServer;
		if(oleTSpan1.GetTotalMinutes() > 30 || oleTSpan2.GetTotalMinutes() > 30)
		{
			::mhMessageBox(NULL, L"��⵽����ϵͳʱ�����׼ʱ���нϴ����⽫Ӱ���ͬ�����ܵ�����ʹ�ã��Ƽ��������޸�!", L"�ƽ��", MB_OK | MB_ICONINFORMATION);
		}
	}
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

LRESULT CAxUI::OnResendVerifyMail(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �ط��ʼ�
	string strParam = "xml=" + CBankData::GetInstance()->m_CurUserInfo.struserid + MY_PARAM_END_TAG;

	string strHWID = GenHWID2();
	wstring wstrHWID = CA2W(strHWID.c_str());
	string strSN = CSNManager::GetInstance()->GetSN();

	/*strParam += strSN;
	strParam += MY_PARAM_END_TAG;

	strParam += strHWID;
	strParam += MY_PARAM_END_TAG;*/
	
	strParam += CBankData::GetInstance()->m_CurUserInfo.strstoken;
	strParam += MY_PARAM_END_TAG;

	CDownloadThread downloadThread;
	downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPSendMailVerify)).c_str(), (LPSTR)strParam.c_str());
	char chTemp[1024] = {0};
	DWORD dwRead = 0;
	if (ERR_SUCCESS != downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead))
	{
		return S_FALSE;
	}

	string strBack = chTemp;
#define RESEND_VERIFY_MAIL "48" // �ʼ��ط��ɹ�
	if(strBack.find(RESEND_VERIFY_MAIL) != string::npos)
	{
		return S_OK;
	}
	else if (strBack.find(MY_MAIL_VERIFY_SUCC) != string::npos)
	{
		// �û��Ѿ���֤���ʼ�
		return atoi(MY_MAIL_VERIFY_SUCC);
	}
	
	return S_FALSE;
}

LRESULT CAxUI::UserAutoLoad(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	string strUserID, strMail, strEtk, strMailVerify;
	USERSTATUS emStatus = emNotLoad;
	CBankData::GetInstance()->ReadNeedAutoLoadUser(strUserID, strMail, strEtk);

	string strParam = "xml=" + strUserID + MY_PARAM_END_TAG;


	string strHWID = GenHWID2();
	wstring wstrHWID = CA2W(strHWID.c_str());
	string strSN = CSNManager::GetInstance()->GetSN();

	/*strParam += strSN;
	strParam += MY_PARAM_END_TAG;

	strParam += strHWID;
	strParam += MY_PARAM_END_TAG;*/

	string strStoken;
	int nStatus = 0;
#define ERR_BUT_NOT_SHOW 10000 // �����������ڽ����ϵ�����ʾ

	if (strEtk.length() > 0)
	{
		// ��Etk���ܳ�stoken
		char chTek[33] = {0};
		string strTekMake = strSN + strHWID;
		UserDataASH256((unsigned char*)strTekMake.c_str(), strTekMake.length(), (unsigned char*)chTek);

		int nLen = (strEtk.length() +1) / 2;

		char* pStoken = new char[nLen + 1];
		char* pUEtk = new char[nLen + 1];
		memset(pStoken, 0, nLen + 1);
		memset(pUEtk, 0, nLen + 1);

		FormatDecVal(strEtk.c_str(), (char*)pUEtk, nLen);

		nLen = UserDataASE256D((unsigned char *)pUEtk, nLen, (unsigned char *)chTek, (unsigned char*)pStoken);

		strStoken = pStoken;
		delete []pUEtk;
		delete []pStoken;

#define SERVER_STOKEN_LENGTH 84 // ��������stoken�ĳ���
		if (nLen <= 0 || strStoken.length() != SERVER_STOKEN_LENGTH)
		{
#define DENCRYPT_PWD_ERR 108
			nStatus = DENCRYPT_PWD_ERR;
		}
	}

	strParam += strStoken;
	strParam += MY_PARAM_END_TAG;


	CDownloadThread downloadThread;
	downloadThread.DownLoadInit(wstrHWID.c_str(), (CHostContainer::GetInstance()->GetHostName(kPAutoLogon)).c_str(), (LPSTR)strParam.c_str());
	char chTemp[1024] = {0};
	DWORD dwRead = 0;
	int nBack = downloadThread.ReadDataFromSever(chTemp, 1024, &dwRead);
	if (ERR_SUCCESS != nBack || 0 == dwRead)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_IE_EXTERNEL, L"�Զ���½ʧ��");

		// ���óɲ���ʾ״̬
		if (0 == nStatus)
			nStatus = ERR_BUT_NOT_SHOW;
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

		// ��ȡ�ʼ���֤��Ϣ
		strRead = strRead.substr(strRead.find(MY_PARAM_END_TAG) + strTag.length(), strRead.length());
		strMailVerify = strRead.substr(0, strRead.find(MY_PARAM_END_TAG));

		// ��KEK EDEKlת����ʵ��ֵ
		char kekVal[33] = {0};
		char edekVal[33] = {0};
		char keyVal[33] = {0};
		int nBack = 0;
		FormatDecVal(strKEK.c_str(), kekVal, nBack);
		FormatDecVal(strEDEK.c_str(), edekVal, nBack);

		UserDataASE256D((unsigned char*)edekVal, 32, (unsigned char*)kekVal, (unsigned char*)keyVal);

		// ʹ��USERID�������ݿ�
		string strData = strUserID;
		strData += ".dat";

		// �����û��Ŀ����óɵ�ǰ���ݿ�,���������
		CBankData::GetInstance()->SetCurrentUserDB((LPSTR)strData.c_str(), keyVal, 32);

		// ֪ͨUI���µ�ǰ�û���
		string strSendUI = strData;
		strSendUI += MY_PARAM_END_TAG;
		string strPWD;
		FormatHEXString(keyVal, 32, strPWD);
		strSendUI += strPWD;
		strSendUI += MY_PARAM_END_TAG;
		::SendMessage(g_hMainFrame, WM_SETTEXT, WM_NOTIFYUI_UPDATE_USER_DB, (LPARAM)strSendUI.c_str());


		// ���ĵ�ǰ�û���Ϣ
		CBankData::GetInstance()->m_CurUserInfo.strstoken = strStoken;
		CBankData::GetInstance()->m_CurUserInfo.strmail = strMail;
		CBankData::GetInstance()->m_CurUserInfo.struserid = strUserID;
		CBankData::GetInstance()->m_CurUserInfo.emUserStatus = emUserNet;
		emStatus = emUserNet;
	}
	else
	{
		// ���MAIL��������Ϣʱ��ʾ��¼ʧ��
#define MAIL_OR_PWD_ERR 106 // ������������
		if (0 == nStatus)
			nStatus = MAIL_OR_PWD_ERR;
	}

	if (nStatus > 0 || strUserID.length() <= 0)
	{
		// �Զ���¼ʧ�ܣ����������
		strUserID = MONHUB_GUEST_USERID;
		strStoken.c_str();
		strMail.c_str();
	}

	// ���µ�ǰ�û���Ϣ��������ʾ��Ϣ
	UpdateUserInfo(strStoken.c_str(), strMail.c_str(), strUserID.c_str(), (int)emStatus, false, (char*)strMailVerify.c_str());

	// ��ʾ������Ϣ
	if (nStatus > 0)
	{
		::PostMessage(g_hMainFrame, WM_SHOW_USER_DLG, nStatus, MY_TAG_LOAD_DLG);
	}

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

		// ֪ͨUI���µ�ǰ�û���
		string strSendUI = "Guest.dat";
		strSendUI += MY_PARAM_END_TAG;
		strSendUI += strPw;
		strSendUI += MY_PARAM_END_TAG;
		::SendMessage(g_hMainFrame, WM_SETTEXT, WM_NOTIFYUI_UPDATE_USER_DB, (LPARAM)strSendUI.c_str());

	// ���鵱ǰ�û��Ƿ����Զ���¼�û�������ǣ������etk�����Զ���¼��״̬ȥ��
	string strUserID = CBankData::GetInstance()->m_CurUserInfo.struserid;	
	string strSQL = "select userid from datUserInfo where userid = '";
	strSQL += strUserID;
	strSQL += "' and autoload = 1";
	string strQueryVal = CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");
	if (strQueryVal.find(strUserID) != string::npos)
	{
		strSQL = "update datUserInfo set autoload = 0, etk = '' where userid = '";
		strSQL += strUserID;
		strSQL += "'";
		CBankData::GetInstance()->QuerySQL(strSQL, "DataDB");
	}

	// ���µ�ǰ�û���Ϣ��������ʾ��Ϣ
	UpdateUserInfo("", "", MONHUB_GUEST_USERID, emNotLoad, false);

	return 0;
}

void CAxUI::UpdateUserInfo(const char* pStoken, const char* pMail, const char* pUserId, const int nUserStatus, bool bChangeMail, char* pVerifyMail)
{
	ATLASSERT(NULL != pStoken && NULL != pMail && NULL != pUserId);
	if (NULL == pStoken || NULL == pMail || NULL == pUserId)
		return;

	// �����ں��û���Ϣ
	CBankData::GetInstance()->m_CurUserInfo.strstoken = pStoken;
	CBankData::GetInstance()->m_CurUserInfo.strmail = pMail;
	CBankData::GetInstance()->m_CurUserInfo.struserid = pUserId;
	CBankData::GetInstance()->m_CurUserInfo.emUserStatus = (USERSTATUS)nUserStatus;
	if (emNotLoad == nUserStatus) // ����û����˳�
		CBankData::GetInstance()->m_CurUserInfo.emChangeStatus = emUserNoChange;


	// ��������ʱ����ΪJS�����Щ����
	if (!bChangeMail)
	{
		// ΪJS��Ӳ���
		CExternalDispatchImpl::AddJSParam("ReloadStart", "1");
		CExternalDispatchImpl::AddJSParam("ReloadTools", "1");
		CExternalDispatchImpl::AddJSParam("ReloadFinance", "1");
		CExternalDispatchImpl::AddJSParam("ReloadSet", "1");
		CExternalDispatchImpl::AddJSParam("ReloadReport", "1");
		CExternalDispatchImpl::AddJSParam("ReloadProduct", "1");
	}

	char userStatus[3] = {0};
	// ֪ͨUI���µ�ǰ�û���Ϣ��ͬ����UI��Ҫʹ����Щֵ��
	string strMesParam = pStoken;
	strMesParam += MY_PARAM_END_TAG;
	strMesParam += pMail;
	strMesParam += MY_PARAM_END_TAG;
	strMesParam += pUserId;
	strMesParam += MY_PARAM_END_TAG;
	if (NULL != pVerifyMail)
		strMesParam += pVerifyMail;
	strMesParam += MY_PARAM_END_TAG;
	strMesParam += itoa(nUserStatus, userStatus, 10);
	strMesParam += MY_PARAM_END_TAG;

	::SendMessage(g_hMainFrame, WM_SETTEXT, WM_UPDATE_USER_STATUS, (LPARAM)strMesParam.c_str());
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


































