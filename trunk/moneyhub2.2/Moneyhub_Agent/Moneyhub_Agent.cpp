// Moneyhub_Agent.cpp : main source file for Moneyhub_Agent.exe
//

#include "stdafx.h"

#include "Moneyhub_Agent.h"
#include "MainDlg.h"
#include "Skin/TuoImage.h"
#include "skin/CoolMessageBox.h"
#include "Config.h"
#include "Security/SecurityCheck.h"
#include "Security/Security.h"
#include "Util.h"
//#include "../USBControl/USBControl.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;


	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);
	dlgMain.UpdateWindow();


	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"����");

	DWORD pid = ::GetCurrentThreadId();
	HRESULT hRes = ::CoInitialize(NULL);
	g_strSkinDir = ::GetModulePath();
	g_strSkinDir += _T("\\Skin\\");
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (_tcsncmp(lpstrCmdLine, _T("-reblack"), 8) == 0)
	{
		HANDLE hEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"MONEYHUBEVENT_BLACKUPDATE");
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"���´���������");
		CGlobalData::GetInstance()->Init();
		int iReturn = (int)_SecuCheckPop.CheckBlackListCache();
		if( hEvent )
		{
			SetEvent(hEvent);
			CloseHandle(hEvent);
		}
		return iReturn;
	}

	if (_tcsncmp(lpstrCmdLine, _T("-rebuild"), 8) == 0)
	{
		HANDLE hEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"MONEYHUBEVENT_WHITEUPDATE");
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"���´���������");
		CGlobalData::GetInstance()->Init();
		int iReturn = (int)_SecuCheckPop.ReBuildSercurityCache();
		if( hEvent )
		{
			SetEvent(hEvent);
			CloseHandle(hEvent);
		}
		return iReturn;
	}

	ThreadCacheDC::InitializeThreadCacheDC();
	ThreadCacheDC::CreateThreadCacheDC();


	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if(CGlobalData::GetInstance()->IsPopAlreadyRunning() == TRUE)
	{
		mhMessageBox(NULL, _T("���ݳ����Ѿ���������..."), L"�ƽ��", MB_OK | MB_SETFOREGROUND);
		return 0;
	}

	ATLASSERT(SUCCEEDED(hRes));

	

	_SecuCheckPop.Start();// ������ȫ��⼰��������
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"������ȫ��⼰��������");

	//CUSBControl::GetInstance()->BeginUSBControl();
	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));


	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"����");

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();

	//::CoUninitialize();

	return nRet;
}




//DWORD WINAPI _threadWaitListCheck(LPVOID lp)
//{
//	//DebugBreak();
//	set<SecCachStruct*> waitlist;
//	CSecurityCache waitCache;
//	waitCache.Init();
//	waitCache.SetAllDataInvalid();
//	HWND frameWnd = (*(HWND*)lp);
//
//	HANDLE mphd=OpenFileMappingW(FILE_MAP_READ,true,L"MoneyHubWaitList");//�����ڴ�ӳ���ļ�
//	if(mphd)
//	{
//		LPVOID lpMapAddr = MapViewOfFile(mphd,FILE_MAP_READ,0,0,0);
//		if(lpMapAddr)
//		{
//			//���б�������ɱ�б��ڴ�ӳ���ļ���
//			DWORD num,i;
//			memcpy(&num,lpMapAddr,sizeof(DWORD));
//			if(num > 100000 || num < 0)
//				return 0;
//
//			char * unPackBuf = (char*)lpMapAddr;
//			unPackBuf += sizeof(DWORD);
//			for( i = 0;i < num;i ++)
//			{
//				SecCachStruct *data = new SecCachStruct;
//				data->tag = 0;
//				memcpy(data->filename,unPackBuf,sizeof(data->filename));
//				unPackBuf += sizeof(data->filename);
//				waitlist.insert(data);
//			}
//			::UnmapViewOfFile(lpMapAddr);
//		}
//		::CloseHandle(mphd);
//	}
//	else 
//		return 0;
//
//	//�Ѿ���ô���ɱ�б��е������ļ�
//	set<wstring> waitfile;
//	set<SecCachStruct*>::iterator site = waitlist.begin();
//	for(;site != waitlist.end();site ++)
//	{
//		wstring fname = (*site)->filename;
//		waitfile.insert(fname);
//	}
//
//	::PostMessageW(frameWnd,WM_CLOUDALARM,0,0);
//	while(1)
//	{
//		//Sleep(1000);
//		Sleep(10*60*1000);//ʮ����
//		//������ʾ�з���
//		::PostMessageW(frameWnd,WM_CLOUDALARM,0,0);
//
//		bool ret = CCloudCheckor::GetCloudCheckor()->Initialize();
//
//		if (ret)
//		{	
//			CCloudCheckor::GetCloudCheckor()->Clear();
//			//��ӽ�����ʾ��Ϣ
//			::PostMessageW(frameWnd,WM_CLOUDCHECK,0,0);
//
//			CCloudCheckor::GetCloudCheckor()->SetFiles(&waitfile,frameWnd);
//			::PostMessageW(frameWnd,WM_CLOUDCHECK,0,0);
//			//������ʾ����ʼ�Ʋ�ɱ
//			ret = CCloudCheckor::GetCloudCheckor()->BeginScanFiles();
//			if(!ret)
//			{
//				CCloudCheckor::GetCloudCheckor()->Uninitialize();
//				//ȡ��������ʾ
//				::PostMessageW(frameWnd,WM_CLOUDNCHECK,0,0);
//				continue;
//			}
//
//			set<wstring>* passfiles = CCloudCheckor::GetCloudCheckor()->GetPassFiles();
//			if(passfiles->size() > 0 )
//			{
//				//��������ɱ�������ļ����뵽��ȫ������
//				for(set<wstring>::iterator tite = passfiles->begin();tite != passfiles->end();tite ++)
//				{
//					SecCachStruct scc;
//
//					wcscpy_s(scc.filename,MAX_PATH,(*tite).c_str());
//					if(GenerateHash((*tite).c_str(),scc.chkdata))
//					{
//						waitCache.Add(scc);
//					}
//				}
//				waitCache.Flush();
//			}
//			//ȡ��������ʾ111
//			set<wstring>* nopass = CCloudCheckor::GetCloudCheckor()->GetUnPassFiles();
//			//nopass->insert(L"D:\\myHiew.dll");
//			if(nopass->size() <= 0)
//			{
//				::PostMessageW(frameWnd,WM_CLOUDNCHECK,0,0);
//				::PostMessageW(frameWnd,WM_CLOUDCLEAR,0,0);				
//				//˵��û�зǷ��ļ�
//				return 0;
//
//			}
//			else
//			{
//				LookUpHash luh;
//				//��û�о�����ɱ�������ļ�����hash�б���
//				for(set<wstring>::iterator tite = nopass->begin();tite != nopass->end();tite ++)
//				{
//					unsigned char chkdata[SECURE_SIZE] = {0};
//
//					if(GenerateHash((*tite).c_str(),chkdata))
//					{
//						luh.AddSecureHash(chkdata);
//					}
//				}
//
//				//���͸��������Ҫ���ص�hash�ļ�
//
//				//���µ�hash�����������������¹���
//				g_moduleHashList.clear();
//				waitCache.GetEigenvalue();
//				CBankLoader::GetInstance()->setSecuModHashBR();
//
//
//				//�ɹ�����ģ���������
//				if(CheckLoadModule(luh))
//				{
//					//��û�н��м��ؿ����ļ�
//					::MessageBox(NULL, _T("�����Ʋ�ɱ��ϵͳ�ڿ��ܴ��ڰ�ȫ���գ��ƽ���Ѿ��Է���ģ���ֹ���أ������ܵ��²ƽ�������е��쳣���������˳��ƽ�����ȫ��ɱ��ɨ�裡"), L"�ƽ�㰲ȫ���", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
//					::PostMessageW(frameWnd,WM_CLOUDNCHECK,0,0);
//					::PostMessageW(frameWnd,WM_CLOUDCLEAR,0,0);
//					return 0;
//
//				}
//				else
//				{
//					MessageBoxW(NULL,L"��⵽�зǷ�ģ����أ�Ϊ��֤��ȫ���ƽ�㽫�˳���",L"�ƽ��",MB_OK| MB_ICONERROR | MB_SETFOREGROUND);
//					SendMessageW(frameWnd,WM_CLOSE,NULL,NULL);
//					exit(-1);
//				}
//			}
//		}
//	}
//}