#include "StdAfx.h"
#include <Shlwapi.h>
#include "ServiceControl.h"

#pragma comment(lib,"shlwapi")

//moneyhub.exe���
extern HANDLE g_terminateHandle;
//ֹͣ�����ʾ
bool          g_isStopService=false;

//���÷��������Լ���������
CServiceControl::CServiceControl(LPCTSTR lpServiceName,LPCTSTR lpServieDescription)
{
	_tcscpy_s(m_tzServiceName,MAX_PATH,lpServiceName);
	_tcscpy_s(m_tzServieDescription,MAX_PATH,lpServieDescription);
	memset(m_tzLogPathName,0,sizeof(m_tzLogPathName));
	//������־·��
	//SetLogFileName(NULL);

}

CServiceControl::~CServiceControl(void)
{
}

// �ж��Ƿ�װ�˷���
bool CServiceControl::IsInstall(void)
{
	SC_HANDLE	hSCM = NULL;
	SC_HANDLE	hService = NULL;

	hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		return false;
	}

	hService = OpenService(hSCM,m_tzServiceName,SERVICE_QUERY_CONFIG);
	if (hService == NULL)
	{
		CloseServiceHandle(hSCM);
		return false;
	}


	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return true;
}

// ��װ����
bool CServiceControl::Install(void)
{
	SC_HANDLE	hSCM = NULL;
	SC_HANDLE	hService = NULL;
	TCHAR		tzFilePathName[MAX_PATH] = {0};

	if (IsInstall())
	{
		return true;
	}
	
	hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		return false;
	}
	SC_LOCK	scLock = LockServiceDatabase(hSCM);

	GetModuleFileName(NULL,tzFilePathName,MAX_PATH);

	hService = CreateService(hSCM,m_tzServiceName,m_tzServiceName,SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,SERVICE_AUTO_START,SERVICE_ERROR_NORMAL,
		tzFilePathName,NULL,NULL,NULL,NULL,NULL);

	if (hService == NULL)
	{
		CloseServiceHandle(hSCM);
		return false;
	}


	//�޸ķ�������
	SERVICE_DESCRIPTION ServiceDescription = {0};
	ServiceDescription.lpDescription = m_tzServieDescription;

	if (0 == ChangeServiceConfig2(hService,SERVICE_CONFIG_DESCRIPTION,&ServiceDescription))
	{
		;
	}


	CloseServiceHandle(hService);

	UnlockServiceDatabase(scLock);
	CloseServiceHandle(hSCM);
	return true;
}

// ��������
bool CServiceControl::Start(void)
{
	SC_HANDLE	hSCM = NULL;
	SC_HANDLE	hService = NULL;
	SERVICE_STATUS	ServiceStatus = {0};
	DWORD		dwWaitTime = 0;


	//�򿪷���
	hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		return false;
	}
	hService = OpenService(hSCM,m_tzServiceName,SERVICE_QUERY_STATUS|SERVICE_START);
	if (hService == NULL)
	{
		CloseServiceHandle(hSCM);
		return false;
	}

	//��ѯ�Ƿ�������
	QueryServiceStatus(hService,&ServiceStatus);
	if (ServiceStatus.dwCurrentState == SERVICE_RUNNING
		|| ServiceStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return true;
	}

	//��������
	if (!StartService(hService,0,NULL))
	{
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return false;
	}

	//��ѯ״̬���Ƿ��Ѿ��������
	QueryServiceStatus(hService,&ServiceStatus);
	dwWaitTime = ServiceStatus.dwWaitHint / 10;
	if (dwWaitTime < 1000)
	{
		dwWaitTime = 1000;
	}
	else if(dwWaitTime > 10000)
	{
		dwWaitTime = 10000;
	}

    //////////////ѭ���ȴ�
	while(ServiceStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		Sleep(dwWaitTime);

		QueryServiceStatus(hService,&ServiceStatus);

	}
    //////////////

	if (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		;
	}
	else
	{
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return false;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return true;
}

// ֹͣ����
bool CServiceControl::Stop(void)
{
	
		SC_HANDLE	hSCM = NULL;
		SC_HANDLE	hService = NULL;
		SERVICE_STATUS	ServiceStatus = {0};
		DWORD		dwWaitTime = 0;

		hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if (hSCM == NULL)
		{
			return false;
		}

		hService = OpenService(hSCM,m_tzServiceName,SERVICE_QUERY_STATUS|SERVICE_STOP);
		if (hService == NULL)
		{
			CloseServiceHandle(hSCM);
			return false;
		}

		//��ѯ�Ƿ�������
		QueryServiceStatus(hService,&ServiceStatus);
		if (ServiceStatus.dwCurrentState == SERVICE_STOPPED
			|| ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCM);
			return true;
		}

		//���ڿ�ʼֹͣ

		if (!ControlService(hService,SERVICE_CONTROL_STOP,&ServiceStatus))
		{
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCM);
			return false;
		}

		//�ٲ�ѯ״̬
		QueryServiceStatus(hService,&ServiceStatus);
		dwWaitTime = ServiceStatus.dwWaitHint / 10;
		if (dwWaitTime < 1000)
		{
			dwWaitTime = 1000;
		}
		else if (dwWaitTime > 10000)
		{
			dwWaitTime = 10000;
		}

		//////////////ѭ���ȴ�
		while (ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			Sleep(dwWaitTime);
			QueryServiceStatus(hService,&ServiceStatus);
		}
		/////////////////////

		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);

		if (ServiceStatus.dwCurrentState == SERVICE_STOPPED)
		{
			return true;
		}
		else
		{
			return false;
		}
	
}

// ж�ط���
bool CServiceControl::UnInstall(void)
{
	if(g_isStopService == true)
	{
		SC_HANDLE hSCM = NULL;
		SC_HANDLE hService = NULL;
		SERVICE_STATUS	ServiceStatus = {0};

		//
		//����Ƿ�װ
		if (!IsInstall())
		{
			return true;
		}

		//��ֹͣ����
		if (!Stop())
		{
			return false;
		}
		//�򿪷���
		hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if (hSCM == NULL)
		{
			return false;
		}

		hService = OpenService(hSCM,m_tzServiceName,SERVICE_STOP|DELETE);
		if (hService == NULL)
		{
			CloseServiceHandle(hSCM);
			return false;
		}

		//��ʼж�ء�
		if (!DeleteService(hService))
		{
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCM);
			return false;
		} 

		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return true;
	}
	return false;
}


// ��ѯ����״̬
DWORD CServiceControl::QueryStatus(void)
{
	SC_HANDLE	hSCM = NULL;
	SC_HANDLE	hService = NULL;
	SERVICE_STATUS	ServiceStatus = {0};

	hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		return false;
	}
	hService = OpenService(hSCM,m_tzServiceName,SERVICE_QUERY_STATUS);
	if (hService == NULL)
	{
		CloseServiceHandle(hSCM);
		return false;
	}

	//��ѯ����״̬
	if (QueryServiceStatus(hService,&ServiceStatus))
	{
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return false;
	}


	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);

	return ServiceStatus.dwCurrentState;
}
