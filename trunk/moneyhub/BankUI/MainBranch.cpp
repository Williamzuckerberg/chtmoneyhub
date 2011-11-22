/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  main.cpp
 *      ˵����  �����̷���ִ�м��������ʵ���ļ�
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.22	���ź�ͨ	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */

#include "StdAfx.h"
#include "MainBranch.h"
#include "UIControl/SecuCheckDlg.h"
//#include "UIControl/InstallCheckDlg.h"
#include "UIControl/CoolMessageBox.h"
#include "Util/SecurityCheck.h"
#include "Util/DriverCommunicator.h"

#include "../Security/BankLoader/export.h"
#include "../Security/BankLoader/DriverLoader.h"
#include "../Utils/HardwareID/genhwid.h"
#include "resource/resource.h"
#include "Version.h"
#include "../Utils/sn/SNManager.h"
#include "Util/Util.h"
#include "Util/Config.h"
#include "UIControl/MainFrame.h"
#include "Tlhelp32.h"
#include "Util/SelfUpdate.h"
#include "../Utils/Config/HostConfig.h"
#include "../Utils/UserBehavior/UserBehavior.h"
using namespace std;
#pragma comment(lib,"Kernel32.lib")

//#include "../Utils/RunLog/RunLog.h"

CMainBranch g_AppBranch;
static HANDLE _hProcessMutex = NULL;	//Ψһ���̱�ʾ���õ��ں˶���
static HANDLE _hIEcoreProcessMutex = NULL;
#define TIMERELAPSE 5000
// IE�ں˽��̺�������BankCore������ʵ��
void RunIECore(LPCTSTR lpstrCmdLine);

CMainBranch::CMainBranch(void)
{
	
}

CMainBranch::~CMainBranch(void)
{
}

bool CMainBranch::UpdateCheck()
{
#ifndef SINGLE_PROCESS
	// ����Ƿ���Ҫ��װ������
	CInstallUpdatePack update;

	if (update.Check())
	{
		update.Setup();
		return false;
	}
	else
	{
		update.setupUpdateWithMH();
	}
#endif

	return true;
}
bool CMainBranch::CheckPop(bool bCheck, HANDLE *pId )
{
#ifndef SINGLE_PROCESS
	if(IsPopAlreadyRunning() == false)
	{
		int ret = IDOK;
		if( bCheck )
			ret = mhMessageBox(NULL,L"�ƽ��ؼ�����δ�������ƽ�㲻�����У�����ȷ�������ƽ��ؼ����̣�ȡ�����˳��ƽ��",L"�ƽ����",MB_OKCANCEL);

		if(ret == IDOK)
		{
			WCHAR wsPath[MAX_PATH] = {0};
			::GetModuleFileNameW(NULL,wsPath,MAX_PATH);
			::PathRemoveFileSpecW(wsPath);

			wstring path(wsPath);
			path += L"\\Moneyhub_Svc.exe";

			STARTUPINFO si;	
			PROCESS_INFORMATION pi;	
			ZeroMemory( &pi, sizeof(pi) );
			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);	
			//��������
			if(CreateProcessW(NULL, (LPWSTR)path.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
			{				
				if( pId )
					*pId = pi.hProcess;
				else
				{
					::CloseHandle( pi.hProcess );		
					::CloseHandle( pi.hThread );
				}
			}
			Sleep(2*1000);//�ȴ�Pop������ȫ����������Ѿ������������͸�����
			return true;
		}
		else
			return false;
	}
#endif

	return true;
}


bool CMainBranch::IsPopAlreadyRunning()
{
	LPCTSTR lpszProcessMutex = _T("_MoneyhubPop_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B9}");
	HANDLE _hProcessMutex  = CreateMutex(NULL, TRUE, lpszProcessMutex);

	DWORD err = GetLastError();
	if(_hProcessMutex != NULL)
		::CloseHandle(_hProcessMutex);
	// ���ں˶����Ѿ����ڻ��߽�ֹ����ʱ��˵���Ѿ������ý���
	if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
	{
		return true;
	}
	return false;
}


bool CMainBranch::SecurityCheck()
{
	//��ȫ��������ʾ
	HANDLE hMutex = CreateMutexW(NULL, FALSE, L"UI_SECURITY_MUTEX");

	//��ȫ���
	CSecuCheckDlg dlg;
	
	if (IDCANCEL == dlg.DoModal(NULL))
	{
		return false;
	}

	if( hMutex )
		::CloseHandle(hMutex);
		

	return true;
}
int	CMainBranch::InstallCheck()
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"���а�װ���");

	CDriverCommunicator cd;
	CSecuCheckDlg dlg(true,true);

	// ��װ��ʱ���Ͱ������ͺ�����������
	DWORD ret = dlg.DoModal();
	if(ret == IDOK)
	{
		cd.sendData();
		cd.SendBlackList();
		return 0;
	}
	return 1;

}
void CMainBranch::StartMonitor()
{
	//��ʱ���HOOK
	if(!(m_returnTimer = SetTimer(NULL,1,TIMERELAPSE,CheckHookProc)))
	{
		 //MessageBoxA(NULL,"set timer is error"," ",MB_OK);
	}
}
void CMainBranch::StopMonitor()
{
	KillTimer(NULL,m_returnTimer);
}	 
	 
	////////////////////////////////////////////////
// ������ʾ��
void CMainBranch::CheckGuide(HWND& hFrame)
{	
	DWORD dwValue;
	DWORD dwType, dwReturnBytes = sizeof(DWORD);

	if (ERROR_SUCCESS == ::SHGetValueW(HKEY_CURRENT_USER, L"Software\\Bank\\Setting",L"IsGuideShow", &dwType, &dwValue, &dwReturnBytes))
	{

		bool isGuideShow = (dwValue != 0);
		if(isGuideShow)
		{
			PostMessage(hFrame,WM_COMMAND,ID_HELP_TIPS,0);
		}
	}
}
bool CMainBranch::GetFramePos(int& nShowWindow,RECT& rcWnd,DWORD& dwMax)
{
	DWORD dwType, dwReturnBytes = sizeof(DWORD), dwPos = 0, dwSize = 0;
	::SHGetValue(HKEY_CURRENT_USER, _T("Software\\Bank"), _T("wndpos"), &dwType, &dwPos, &dwReturnBytes);
	::SHGetValue(HKEY_CURRENT_USER, _T("Software\\Bank"), _T("wndsize"), &dwType, &dwSize, &dwReturnBytes);
	::SHGetValue(HKEY_CURRENT_USER, _T("Software\\Bank"), _T("wndmax"), &dwType, &dwMax, &dwReturnBytes);

	if (dwSize != 0)
	{
		rcWnd.left = LOWORD(dwPos);
		rcWnd.top = HIWORD(dwPos);
		rcWnd.right = rcWnd.left + LOWORD(dwSize);
		rcWnd.bottom = rcWnd.top + HIWORD(dwSize);

		HMONITOR hMon = ::MonitorFromRect(&rcWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(MONITORINFO) };
		::GetMonitorInfo(hMon, &mi);
		RECT rcTest = { mi.rcMonitor.left + 10, mi.rcMonitor.top + 10, mi.rcMonitor.right - 10, mi.rcMonitor.bottom - 10 };
		RECT rcIntersect;
		if (!::IntersectRect(&rcIntersect, &rcTest, &rcWnd))
		{
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWnd, 0);
			rcWnd.left += 50; rcWnd.top += 50;
			rcWnd.right -= 50; rcWnd.bottom -= 50;
		}
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWnd, 0);
		rcWnd.left += 50; rcWnd.top += 50;
		rcWnd.right -= 50; rcWnd.bottom -= 50;

		nShowWindow = SW_SHOWMAXIMIZED;
	}
	return true;
}
VOID CALLBACK CMainBranch::CheckHookProc(HWND hwnd , UINT uMsg , UINT_PTR idEvent , DWORD dwTime)
{
	CDriverLoader::CheckDriverImagePath(true);	
	BankLoader::checkHook();
}

bool CMainBranch::PopSetPage()
{
	HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
	if (hWnd)
	{
		::PostMessage(hWnd, WM_SWITCHTOPPAGE, (WPARAM)kToolsPage, 0);
		return true;
	}
	else
	{
		SetTopPage(kToolsPage);
		return false;
	}

}

bool CMainBranch::Shell(LPCTSTR lpstrCmdLine)
{
	wstring path(lpstrCmdLine + 14);

	::ShellExecute(NULL, _T("open"), path.c_str(), NULL, NULL, SW_SHOWNORMAL);
	return true;
}
bool CMainBranch::RunUAC(LPCTSTR lpstrCmdLine)
{
	OSVERSIONINFOEX OSVerInfo; 
	OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX); 
	if(!GetVersionEx((OSVERSIONINFO *)&OSVerInfo)) 
	{ 
		OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); 
		GetVersionEx((OSVERSIONINFO *)&OSVerInfo); 
	} 

	if(OSVerInfo.dwMajorVersion >= 6) // Vista ���� 
	{ 
		HWND hMainFrame = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
		if(hMainFrame != NULL)
		{
			::CloseWindow(hMainFrame);
		}

		USES_CONVERSION;

		string aid(W2A(lpstrCmdLine + 10));

		WCHAR szPath[MAX_PATH] ={ 0 };
		::GetModuleFileName(NULL, szPath, _countof(szPath));
		::PathRemoveFileSpecW(szPath);

		wstring path(szPath);
		path += L"\\Moneyhub.exe";

		SHELLEXECUTEINFOW shExecInfo ;    
		memset(&shExecInfo,0,sizeof(SHELLEXECUTEINFOW));    
		shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);    
		shExecInfo.lpVerb = L"runas";    
		shExecInfo.lpFile = path.c_str();    
		shExecInfo.lpParameters = L"-uac";    
		shExecInfo.nShow = SW_SHOW ;    
		shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS ;   
		shExecInfo.hInstApp = NULL;
		//::MessageBoxW(NULL, L"ShellExecuteEx", L"�ƽ��UAC", MB_OK);
		if(ShellExecuteEx(&shExecInfo)) 
		{ 
			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_BANK_UAC, L"RunUAC �ɹ�");
			if(shExecInfo.hProcess != 0)
			{
				::WaitForSingleObject(shExecInfo.hProcess, 5000);
			}			
			// �����ɹ� 
		} 
		else 
		{ 
			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_BANK_UAC, L"RunUACʧ��");
			// ����ʧ�ܣ�����UACû�л���û���� 
			//::MessageBoxW(NULL, L"ȡ���ղ�",L"�ƽ��", MB_OK);
			HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
			if (hWnd)
			{
				if(aid.size() > 0)
				{
					LPARAM lParam = 0;
					memcpy((void *)&lParam, aid.c_str(), sizeof(LPARAM));
					::PostMessageW(hWnd, WM_CANCEL_ADDFAV, 0, lParam);//
				}
			}
		}

		hMainFrame = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
		if(hMainFrame != NULL)
		{
			::SwitchToThisWindow(hMainFrame, true);
		}
	}
	return true;
}
void CMainBranch::RunIECoreProcess(LPCTSTR lpstrCmdLine)
{
	LPCTSTR lpszProcessMutex = _T("_Moneyhub_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B7}");
	_hIEcoreProcessMutex  = CreateMutex(NULL, TRUE, lpszProcessMutex);

	// gao 2010-12-16 ��listmanager��ȡ�ں˺�UI���ݷֿ�
	CListManager::Initialize(true);

	RunIECore(lpstrCmdLine + 3);
	::CloseHandle(_hIEcoreProcessMutex);
}

bool CMainBranch::CheckIECoreProcess()
{
	LPCTSTR lpszProcessMutex = _T("_Moneyhub_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B7}");
	_hIEcoreProcessMutex  = CreateMutex(NULL, TRUE, lpszProcessMutex);

	DWORD err = GetLastError();
	if(_hIEcoreProcessMutex != NULL)
		::CloseHandle(_hIEcoreProcessMutex);

	// ���ں˶����Ѿ����ڻ��߽�ֹ����ʱ��˵���Ѿ������ý���
	if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
	{
		return true;
	}

	return false;
}
bool CMainBranch::CheckToken()
{
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	BOOL bRestricted = IsTokenRestricted(hToken);
	::CloseHandle(hToken);
	return (bRestricted == TRUE) ? false : true;
}
bool CMainBranch::InitManagerList()
{
	//CListManager::Initialize(false);
	if(! CListManager::_()->GetResult())
	{
		mhMessageBox(NULL,L"�ƽ���ȡ�ؼ��ļ�ʧ�ܣ��볢����������ϵͳ�������°�װ�ƽ��",L"�ƽ����",MB_OK);
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_COMMON_ERROR, L"ListManager��ȡʧ��");
		return false;
	}
	return true;
}

bool CMainBranch::TerminateIECore()
{
	HANDLE hProcessToken = NULL;
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcessToken)) 
	{ 
		return false; 
	}

	TOKEN_PRIVILEGES tp={0};
	LUID luid={0};  
	if(!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&luid))  
	{ 
		return false; 
	}  
	tp.PrivilegeCount = 1;  
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  

	// Enable the privilege
	AdjustTokenPrivileges(hProcessToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL);  

	if(GetLastError() != ERROR_SUCCESS)  
	{
		return false;  
	}
	::CloseHandle(hProcessToken);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hSnap == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32W pew = {sizeof(PROCESSENTRY32W)};
	bool bHaveOther=false;
	Process32FirstW(hSnap,&pew);

	wstring moneyhub = L"moneyhub.exe";//�ҵ�moneyhub����
	wchar_t buf[500] = {0},tmp[500]={0} ;
	HANDLE ownH = GetCurrentProcess();

	do{
		wstring wExe(pew.szExeFile);
		transform(wExe.begin(), wExe.end(), wExe.begin(), tolower);

		if(wExe == moneyhub)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,false,pew.th32ProcessID);
			if(hProcess)
			{
				if(ownH != hProcess)
				{
					::TerminateProcess(hProcess,0);
				}			
				::CloseHandle(hProcess);
			}
		}

	}while(Process32NextW(hSnap,&pew));

	::CloseHandle(ownH);
	::CloseHandle(hSnap);
	return bHaveOther;	
}
// �ж�MoneyHub�������в��л��������ĺ���
BOOL CMainBranch::IsAlreadyRunning()
{
	LPCTSTR lpszProcessMutex = _T("_Moneyhub_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B6}");
	_hProcessMutex  = CreateMutex(NULL, TRUE, lpszProcessMutex);

	DWORD err = GetLastError();
	// ���ں˶����Ѿ����ڻ��߽�ֹ����ʱ��˵���Ѿ������ý���
	if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
	{
		//������
		HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
		if (hWnd)
		{
			SwitchToThisWindow(hWnd, true);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0 ,SWP_NOMOVE | SWP_NOSIZE);
			return TRUE;
		}
		// ��ȫ������
		HWND hSwnd = FindWindowW (NULL, SECU_DLG_TITLE);
		if(hSwnd&& IsWindowVisible(hSwnd))
		{
			// �����н���ŵ����м������ʾ
			SwitchToThisWindow(hSwnd, true);
			return TRUE;
		}
		// ����ƽ�������
		hWnd = FindWindow(NULL, _T("�ƽ������"));
		if (hWnd && IsWindowVisible(hWnd))
		{
			SwitchToThisWindow(hWnd, true);
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}
void CMainBranch::CloseHandle()
{
	::CloseHandle(_hProcessMutex);
}
int CMainBranch::Install()
{
	DWORD dwType;
	DWORD dwValue;
	DWORD dwReturnBytes = sizeof(DWORD);

	if (ERROR_SUCCESS != ::SHGetValueW(HKEY_CURRENT_USER, L"Software\\Bank\\Setting",L"IsGuideShow", &dwType, &dwValue, &dwReturnBytes))
	{
		DWORD i = 1;
		::SHSetValueW(HKEY_CURRENT_USER,L"Software\\Bank\\Setting",L"IsGuideShow",REG_DWORD,&i,4);//������ʾʹ����
	}

	DWORD i = 31;
	::SHSetValueW(HKEY_CURRENT_USER,L"Software\\Bank\\Setting",L"Version",REG_DWORD,&i,4);//����moneyhub�汾��


	// ����ie8�ı�׼ģʽ���вƽ�㣬����Ĭ�ϵ�ie8��IE7����ģʽ
	int iever = GetIEVersion();
	if(iever == 8)
	{
		DWORD iemode = 0x1F40;
		SHSetValueW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION", L"Moneyhub.exe", REG_DWORD, &iemode, sizeof(DWORD));
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�ƽ������IE8");
	}
	else if(iever == 9)
	{
		DWORD iemode = 0x2328;
		SHSetValueW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION", L"Moneyhub.exe", REG_DWORD, &iemode, sizeof(DWORD));
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�ƽ������IE9");
	}

	// ��װ����������
	if(BankLoader::InstallAndStartDriver())
	{
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"��װ�����ɹ�");
		return 0;
	}

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"��װ����ʧ��");

	return 1;
}
DWORD CMainBranch::GetIEVersion()
{
	const TCHAR szFilename[] = _T("mshtml.dll");   
	DWORD dwMajorVersion =0;   
 
	DWORD dwHandle = 0;   
	DWORD dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwHandle);   
	if (dwVerInfoSize)   
	{   
		LPVOID lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);   
		if (lpBuffer)   
		{   
			if (GetFileVersionInfo(szFilename, dwHandle, dwVerInfoSize, lpBuffer))   
			{   
				VS_FIXEDFILEINFO * lpFixedFileInfo = NULL;   
		        UINT nFixedFileInfoSize = 0;   
				if (VerQueryValue(lpBuffer, TEXT("\\"), (LPVOID*)&lpFixedFileInfo, &nFixedFileInfoSize) &&(nFixedFileInfoSize))   
				{   
					dwMajorVersion = HIWORD(lpFixedFileInfo->dwFileVersionMS);     
  
				}   
			}   
			LocalFree(lpBuffer);  
		}
	}   
	
	return dwMajorVersion;
}

int CMainBranch::UnInstall()
{
	// HardwareID & Version;
	TCHAR szTempFile[MAX_PATH + 1];
	GetTempPath(MAX_PATH, szTempFile);
	_tcscat_s(szTempFile, _T("A9BD62759DBE4df1B7F7F619F99F17FB"));
	HANDLE hFile = ::CreateFile(szTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		char szBuf[1024];
		memset(szBuf, 0, sizeof(szBuf));
		USES_CONVERSION;
		sprintf_s(szBuf, sizeof(szBuf), "%suninstall.php?MoneyhubUID=%s&v=%s&SN=%s",W2CA(CHostContainer::GetInstance()->GetHostName(kWeb).c_str()), GenHWID2().c_str(), ProductVersion_All,CSNManager::GetInstance()->GetSN().c_str());

		DWORD dw;
		WriteFile(hFile, szBuf, strlen(szBuf), &dw, NULL);
		::CloseHandle(hFile);
	}



	if(BankLoader::UnInstallDriver())
	{
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"ж�������ɹ�");

		return 0;
	}
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"ж������ʧ��");

	return 1;
}


void CMainBranch::GenerationBlackCache()
{
	WCHAR wcsBuf[MAX_PATH] = {0};
	GetModuleFileNameW(NULL, wcsBuf, MAX_PATH);
	PathRemoveFileSpecW(wcsBuf);
	std::wstring wstrBuf = wcsBuf;
	wstrBuf += L"\\Moneyhub_Svc.exe";  
	

	ShellExecuteW(GetDesktopWindow(), L"open", wstrBuf.c_str(), L"-reblack", NULL, SW_HIDE);
}

void CMainBranch::CheckHID()
{
	if(g_strHardwareId.size() < 32)
	{
		char gid[ 33 ] = {0};
		int gleft = (int)32 - g_strHardwareId.size();
		for(int i = 0; i < gleft; i ++)
			gid[i] =  'F';//
		g_strHardwareId += gid;
	}
}
void CMainBranch::RenameChk()
{
	char wsPath[MAX_PATH] = {0};
	::GetModuleFileNameA(NULL,wsPath,MAX_PATH);
	::PathRemoveFileSpecA(wsPath);

	string wsDirectory = wsPath;

	wsDirectory += "\\BankInfo\\banks\\";

	string  dir = wsDirectory + "*.*";

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(dir.c_str(), &FindFileData);

	if(INVALID_HANDLE_VALUE == hFind)
		return;
	do{
		std::string fn = FindFileData.cFileName;

		if ((fn !=  ".") && (fn != "..") && (fn != ".svn"))
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				string indir = wsDirectory + fn + "\\*.chk";
				WIN32_FIND_DATAA FindFileData2;
				HANDLE hFind2;

				hFind2 = FindFirstFileA(indir.c_str(), &FindFileData2);

				if(INVALID_HANDLE_VALUE != hFind2)
				{
					do{
						std::string fn2 = FindFileData2.cFileName;
						string oldname = wsDirectory + fn + "\\" + fn2;
						DeleteFileA(oldname.c_str());

					}while (FindNextFileA(hFind2, &FindFileData2) != 0);

					FindClose(hFind2);
				}
			}
		}
	}while (FindNextFileA(hFind, &FindFileData) != 0);

	FindClose(hFind);

}