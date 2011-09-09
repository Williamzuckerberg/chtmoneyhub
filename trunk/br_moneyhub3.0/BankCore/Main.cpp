/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  Main.cpp
*      ˵����  ie�ں˽����еĺ����̺߳����ļ�
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2010.11.17	���ź�ͨ
*-----------------------------------------------------------*
*/
#include "stdafx.h"
#include "../Security/BankProtector/export.h"
#include "ProcessMonitor/RegMonitor.h"
#include "AxUI.h"
#include "AxHookManager.h"
#include "WebBrowserCore.h"
#include "AxControl.h"
#include "LookUpHash.h"
#include "InternetSecurityManagerImpl.h"
#include "DownloadManagerImpl.h"
#include "../BankUI/Util/CleanHistory.h"
#include "../Utils/ListManager/ListManager.h"
#include "..\BankUI\Util\ProcessManager.h"
#include "..\Utils\ExceptionHandle\ExceptionHandle.h"
#include "GetBill\BillUrlManager.h"
DWORD WINAPI _threadCheckState(LPVOID lpParameter);
HWND g_hMainFrame = NULL;
/**
*  ie�ں��̺߳���
* @param lpszCmdline ie�ں˽������õĸ����ھ��
*/
void RunIECore(LPCTSTR lpszCmdline)
{
	
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"����RunIECore�߳�");


	CExceptionHandle::MapSEtoCE();
	CBillUrlManager::GetInstance()->Init();
		// ����ʷ��Ϣ�Ķ�д�ļ��ӿڽ��йҹ�
		//CListManager::Initialize(false);
	if(! CListManager::_()->GetResult())
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_INIT, L"�ں�У��ListManagerʧ��");
		return;
	}

	BankProtector::Init();
	// ���ý����ڵ������ڹ�������
	CoInternetSetFeatureEnabled(FEATURE_WEBOC_POPUPMANAGEMENT,SET_FEATURE_ON_PROCESS,TRUE);

	CoInitialize(NULL);

	// ��ʼ��GDI����
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Register Common Controls Class
	::AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

	// Initialize
	//::DebugBreak();
	CRegKeyManager::Initialize();

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"RegKeyManager��ʼ��");


	// �Ը�ieҳ�����Ϣ�Ĵ�����йҹ�
	CAxHookManager::Initialize();
	CInternetSecurityManagerImpl::Initialize();

	HWND hMainFrame = (HWND)_ttoi(lpszCmdline);	
	g_hMainFrame = hMainFrame; // gao

	DWORD dwThreadID;
	// ���������̣߳������߳�
#ifndef SINGLE_PROCESS
	::CloseHandle(::CreateThread(NULL, 0, _threadCheckState, (LPVOID)&hMainFrame, NULL, &dwThreadID));
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"��������߳�");
#endif

	CAxUI axui;

	::SendMessage(hMainFrame, WM_MULTI_PROCESS_NOTIFY_AXUI_CREATED, 0, (LPARAM)axui.m_hWnd);
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"WM_MULTI_PROCESS_NOTIFY_AXUI_CREATED���");
	// ��axui������浽CprocessManager��
	CProcessManager::_()->OnAxUICreated (axui.m_hWnd);
	

	// ��Ϣѭ�����ȴ��´��ڴ�������Ϣ
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	
	try
	{
		CoUninitialize();
	}

	catch(CExceptionHandle eH)
	{
		eH.SetThreadName ("RunIECore Thread Error");
		eH.RecordException ();
	}
}
DWORD WINAPI _threadCheckState(LPVOID lpParameter)
{

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"�������߳�");

	HWND hMainFrame = (*(HWND*)(lpParameter));

	DWORD dwMainProcessId = 0;
	::GetWindowThreadProcessId(hMainFrame, &dwMainProcessId);

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwMainProcessId);
	if (hProcess == NULL)
	{
		CleanHistory();
		::TerminateProcess(::GetCurrentProcess(), 1);
	}

	HANDLE _hMainProcessMutex = NULL;
	DWORD dwExitCode = 0;
	while(1)
	{
		LPCTSTR lpszProcessMutex = _T("_Moneyhub_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B6}");
		_hMainProcessMutex  = OpenMutexW(NULL, TRUE, lpszProcessMutex);

		DWORD err = GetLastError();
		if(_hMainProcessMutex != NULL)
			::CloseHandle(_hMainProcessMutex);

		// ���ں˶����Ѿ����ڻ��߽�ֹ����ʱ��˵������ܽ��̻���
		if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
		{
			Sleep(1300);
		}
		else // �������ܽ��̱�ɱ��
		{
			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"����̹߳ر��ں˽���");
			CleanHistory();
			::TerminateProcess(::GetCurrentProcess(), 1);
		}
	}


	return 0;
}


//////////////////////////////////////////////////////////////////////////
/**
*  ÿһ��ieҳ�洦���̺߳���
* @param lpParameter �����ھ��
*/
DWORD WINAPI ThreadProcCreateAxControl(LPVOID lpParameter)
{
	CExceptionHandle::MapSEtoCE();

	HWND hChildFrame = (HWND) lpParameter;
	CAxHookManager hook;
	hook.Hook();

	CAxControl *pAxControl = new CAxControl(hChildFrame);
	RECT rcClient;
	::GetClientRect(hChildFrame, &rcClient);
	// ����iwebBrowser����������
	pAxControl->Create(hChildFrame, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);

	pAxControl->CreateIEServer();
	// ֪ͨ�����ڴ������
	::PostMessage(hChildFrame, WM_ITEM_NOTIFY_CREATED, 0, (LPARAM)pAxControl->m_hWnd);

	MSG msg;
	// ��Ϣѭ��������������������Ҽ���������ر�
	while (::GetMessage(&msg, NULL, 0, 0))
	{	
		if (msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP || msg.message == WM_RBUTTONDBLCLK)
			continue;
		else if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
		{
			BOOL bCtrlKey = GetKeyState(VK_CONTROL) & 0x8000;
			if (msg.message == WM_KEYDOWN && bCtrlKey && msg.wParam == VK_TAB)
			{
				::PostMessage(hChildFrame, WM_ITEM_TOGGLE_CATECTRL, 0, 0);
				continue;
			}
			if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP)/*&& ((!bCtrlKey && msg.wParam == VK_TAB) || 
				(msg.wParam == VK_F5) || 
				(bCtrlKey && msg.wParam == 'C') || 
				(bCtrlKey && msg.wParam == 'V') || 
				(bCtrlKey && msg.wParam == 'A') || 
				(bCtrlKey && msg.wParam == 'X') ))*/
			{
				CComQIPtr<IOleInPlaceActiveObject, &__uuidof(IOleInPlaceActiveObject)> spInPlaceActiveObject(pAxControl->m_pCore->m_pWebBrowser2);
				if (spInPlaceActiveObject && spInPlaceActiveObject->TranslateAccelerator(&msg) == S_OK)
					continue;
			}

		}

		else if (WM_CLOSE == msg.message)
		{
			TCHAR szMsgHwndClassName[300];
			::GetClassName(msg.hwnd, szMsgHwndClassName, _countof(szMsgHwndClassName));
			if (_tcscmp(szMsgHwndClassName, _T("Shell Embedding")) == 0)
			{
				::PostMessage(hChildFrame, WM_CLOSE, 0, 0);
				continue;
			}
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	try
	{
		::OleUninitialize();
		hook.Unhook();
	}

	catch(CExceptionHandle eH)
	{
		eH.SetThreadName ("ThreadProcCreateAxControl Thread Error");
		eH.RecordException ();
	}
	return 0;
}


