#include "stdafx.h"
#include "USBSoftWareDetector.h"
#include "Tlhelp32.h"
#include <algorithm>

#pragma once

CUSBSoftWareDetector* CUSBSoftWareDetector::m_Instance = NULL;

CUSBSoftWareDetector* CUSBSoftWareDetector::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CUSBSoftWareDetector();
	return m_Instance;
}

CUSBSoftWareDetector::CUSBSoftWareDetector()
{
}
CUSBSoftWareDetector::~CUSBSoftWareDetector()
{
}

bool CUSBSoftWareDetector::CheckFile(list<wstring>& fname)							// ����ļ��Ƿ������Ϊ��׼
{
	list<wstring>::iterator ite = fname.begin();
	for(;ite != fname.end(); ite ++)
	{
		if(::PathFileExistsW(ite->c_str()) == FALSE)
			return false;
	}
	return true;

}
bool CUSBSoftWareDetector::CheckRegInfo(list<RegInfo>& reginfo)		// ���ע����Ƿ����
{

	list<RegInfo>::iterator ite = reginfo.begin();
	for(;ite != reginfo.end(); ite ++)
	{
		HKEY hKey = NULL;
		if (ERROR_SUCCESS != ::RegOpenKeyExW(ite->rootkey,ite->subkey.c_str(),NULL, KEY_QUERY_VALUE,&hKey))
		{
			return false;
		}
		if(hKey)
			::RegCloseKey(hKey);
	}
	return true;
}

bool CUSBSoftWareDetector::CheckUSBSoftWare(USBSoftwareInfo& softinfo, bool bNeedRepair, bool bNeedRestart)//��Ⲣ�޸�software���������
{
	if(!bNeedRepair)//����Ҫ�޸�
	{
		if(CheckFile(softinfo.files) && CheckKeyFile(softinfo.programinfo, bNeedRepair) && CheckRegInfo(softinfo.reginfo))
			return true;
		return false;	
	}
	else
		CheckKeyFile(softinfo.programinfo, bNeedRepair,bNeedRestart);
	return true;
}

bool CUSBSoftWareDetector::CheckKeyFile(std::map<std::wstring,ProgramType>& programinfo, bool bNeedRepair, bool bNeedRestart)
{
	std::map<wstring, ProgramType>::iterator ite = programinfo.begin();
	for(; ite != programinfo.end() ; ite ++)
	{
		int ret = true;
		if((*ite).second == pNormal && bNeedRepair)
			ret = CheckProgramIsRunning((*ite).first , bNeedRestart);
		else if((*ite).second == pDriver)
			ret = CheckIsDriverInstalled((*ite).first);
		else if((*ite).second == pService)
			ret = CheckServiceIsWork((*ite).first, bNeedRepair);

		if(ret == false)
			return false;
	}
	return true;	
}
bool CUSBSoftWareDetector::CheckIsDriverInstalled(const wstring& dname)
{
	SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL)
	{
		int error = GetLastError();
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckDriver OpenSCManager ʧ��:%d", error));
		return false;
	}

	SC_HANDLE hSCService = OpenServiceW(hSCManager, dname.c_str(), SERVICE_ALL_ACCESS);
	if (hSCService != NULL) 
	{
		int error = GetLastError();
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckDriver OpenService %sʧ��::%d",dname.c_str(), error));
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return true;
	}

	CloseServiceHandle(hSCManager);
	return false;
}

bool CUSBSoftWareDetector::CheckServiceIsWork(const wstring& sname, bool bNeedRepair)
{
	SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL)
	{
		int error = GetLastError();
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckService OpenSCManager ʧ��:%d", error));
		return false;
	}

	SC_HANDLE hSCService = OpenServiceW(hSCManager, sname.c_str(), SERVICE_ALL_ACCESS);
	if (hSCService == NULL) 
	{
		int error = GetLastError();
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckService OpenService %s ʧ��:%d", sname.c_str(), error));

		CloseServiceHandle(hSCManager);
		return false;
	}
	if(!bNeedRepair)//����Ҫ�޸���ֱ�ӷ�����ȷ
	{
		CloseServiceHandle(hSCService);
		CloseServiceHandle(hSCManager);
		return true;
	}


	// ��ѯ����״̬
	bool scret = false;
	SERVICE_STATUS ss;
	memset(&ss, 0, sizeof(ss));
	BOOL bret = QueryServiceStatus(hSCService,	&ss);
	if(bret)
	{
		DWORD dwErr = ss.dwWin32ExitCode;
		DWORD dwState = ss.dwCurrentState;

		if (dwState == SERVICE_RUNNING)
		{                 
			// ����������
			scret = true;
		}
		else
		{
			LPQUERY_SERVICE_CONFIG ServicesInfo = NULL;
			ServicesInfo = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 64*1024);//�����㹻���ڴ�

			DWORD nRequire=0;

			if(!::QueryServiceConfigW(hSCService,ServicesInfo, 64*1024,&nRequire))
			{
				int error = GetLastError();
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckService QueryServiceConfig  %s ʧ��:%d", sname.c_str(), error));
				LocalFree(ServicesInfo);
				CloseServiceHandle(hSCService);
				CloseServiceHandle(hSCManager);
				return scret;
			}
			// ����÷��񱻽��ã���ô��Ϊ�ֶ�
			if(ServicesInfo->dwStartType == SERVICE_DISABLED)
			{
				SC_LOCK sclLock;    
				sclLock = LockServiceDatabase(hSCService);
				if (! ChangeServiceConfig(hSCService, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE, NULL,    
					NULL, NULL, NULL, NULL, NULL, NULL) )                // display name: no change   
				{
					int error = GetLastError();
					CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckService ChangeServiceConfig  %s ʧ��:%d", sname.c_str(), error));
					UnlockServiceDatabase(sclLock);
					LocalFree(ServicesInfo);
					CloseServiceHandle(hSCService);
					CloseServiceHandle(hSCManager);
					return scret;
				}
				UnlockServiceDatabase(sclLock);
			}
			LocalFree(ServicesInfo);   

			// �����÷���
			bret = StartService(hSCService, 0, NULL);
			if(bret) 
			{
				scret = true;
			}
			else
			{
				int error = GetLastError();
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckService StartService  %s ʧ��:%d", sname.c_str(), error));
			}
		}
	}

	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);
	return scret;	
}

bool CUSBSoftWareDetector::CheckProgramIsRunning(const wstring& pname, bool bNeedRestart)//ȫ��д�ĳ����ļ�����
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"Begin Repair Program");
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, pname.c_str());
	if(pname.c_str() == L"")
	{
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"CheckProgramIsRunning NULL");
		return true;
	}
	GetPriviledge();
	
	size_t tsize = pname.find_last_of('\\');
	if(tsize == std::wstring::npos)
		return true;

	wstring program = pname.substr(tsize + 1, pname.size() - tsize - 1);//��ȫ·�������н�ȡ���һ�εĳ�������

	HANDLE hProcessSnap = NULL;
	BOOL   bRet         = FALSE;
	PROCESSENTRY32 pe32 = {0};

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		int error = GetLastError();
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"CheckProgramIsRunning CreateToolhelp32Snapshotʧ��:%d",error));
		return FALSE;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);


	if(Process32First(hProcessSnap,&pe32))
	{
		do
		{
			wstring exename(pe32.szExeFile);
			transform(exename.begin(), exename.end(), exename.begin(), towupper);
			if(exename == program)
			{
				if(!bNeedRestart)
				{
					CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"exename :%s;program:%s", exename.c_str(),program.c_str()));
					CloseHandle(hProcessSnap);
					return true;
				}
				else
				{
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,false,pe32.th32ProcessID);
					if(hProcess)
					{
						CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"Terminate;program:%s", exename.c_str()));
						::TerminateProcess(hProcess,0);
						::CloseHandle(hProcess);
					}
				}
				break;
			}
		}while(Process32Next(hProcessSnap,&pe32));


		// �Ƿ�Ӧ�������ý���
		STARTUPINFO si;	
		PROCESS_INFORMATION pi;	
		ZeroMemory( &pi, sizeof(pi) );	
		ZeroMemory( &si, sizeof(si) );	
		si.cb = sizeof(si);	
		// �򿪽���	
		if(CreateProcessW(NULL, (LPWSTR)pname.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{	
			CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"�޸����̳ɹ�");
			CloseHandle(pi.hProcess);		
			CloseHandle(pi.hThread);	
			return true;
		}
		else
		{
			DWORD error = GetLastError();
			CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, CRecordProgram::GetInstance()->GetRecordInfo(L"�޸�����%sʧ��:%d", pname.c_str(), error));
		}
		return false;
	}

	CloseHandle(hProcessSnap);
	return false;

}


bool CUSBSoftWareDetector::GetPriviledge()
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
	return true;

}


