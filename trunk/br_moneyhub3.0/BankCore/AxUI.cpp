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
#include <vector>
#include <algorithm>
using namespace std;

CAxUI::CAxUI() :m_hTag(NULL)
{
	Create(HWND_MESSAGE); // call WM_CREATE
}


//////////////////////////////////////////////////////////////////////////

int CAxUI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
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
	CUserBehavior::GetInstance ()->Action_SendDataToServerWhenExit (); // �������ݵ���������

	
	//CBankData::GetInstance()->ExitWriteCurUserTpFile2Db();

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
//}