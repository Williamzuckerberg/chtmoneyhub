// StartTimeService.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Windows.h>
#include <Shlwapi.h>
#include "ServiceControl.h"

#pragma comment(lib,"shlwapi")


//////////////////////////////////////////////////////////////////////////export
TCHAR	tzServiceName[] = _T("MoneyHubService");
TCHAR	tzServiceDescription[] = _T("���ٲƽ�������ٶȣ��رո÷�������Ӱ��ƽ��������ٶȣ�");
//////////////////////////////////////////////////////////////////////////


SERVICE_STATUS	ServiceStatus = {0};
SERVICE_STATUS_HANDLE	hStatus = NULL;
HANDLE	hEventEnd = NULL;	//�����Ƿ�����¼�
HANDLE  g_terminateHandle=NULL;
extern bool   g_isStopService;

//����������
void WINAPI ServiceMain();
void WINAPI ServiceCtrlHandle(DWORD dwOpcode);
//�̺߳���
DWORD	WINAPI ThreadMain(LPVOID pParam);

CServiceControl CSerCtrl(tzServiceName,tzServiceDescription);;

int _tmain(int argc, LPTSTR argv);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	if( (_tcsncmp(lpstrCmdLine, _T("install"), 7) == 0) 
		|| (_tcsncmp(lpstrCmdLine, _T("uninstall"), 9) == 0) 
		|| (_tcsncmp(lpstrCmdLine, _T("start"), 5) == 0)
		|| (_tcsncmp(lpstrCmdLine, _T("stop"), 4) == 0)  )

		return _tmain(2,lpstrCmdLine);

	else
		return _tmain(1,lpstrCmdLine);
}


int _tmain(int argc, LPTSTR argv)
{
	if (argc == 2)
	{
		//��װ
		if (_tcsicmp(argv,_T("install")) == 0 )
		{
			if (CSerCtrl.Install())
			{
				return 1;
			}
		}
		//ж��
		if (_tcsicmp(argv,_T("uninstall")) == 0 )
		{
			g_isStopService=true;
			if (CSerCtrl.UnInstall())
			{
				return 1;
			}
		}
		//����
		if (_tcsicmp(argv,_T("start")) == 0 )
		{
			if (CSerCtrl.Start())
			{
				return 1;
			}
		}

		//ֹͣ
		if (_tcsicmp(argv,_T("stop")) == 0 )
		{
			if (CSerCtrl.Stop())
			{
				return 1;
			}
		}

	}
	else
	{
		//��������
		SERVICE_TABLE_ENTRY ServiceTable[] = 
		{
			{tzServiceName,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
			{NULL,NULL}
		};

		if (StartServiceCtrlDispatcher(ServiceTable) == 0)
		{
			return 0;
		}
	}

	return 0;
}

void WINAPI ServiceMain()
{
	HANDLE	hThread = NULL;
	DWORD	dwThreadID = 0;

	//DebugBreak();
	//��ʼ��
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwServiceType = SERVICE_WIN32;

	//���Handler������ע��
	hStatus = RegisterServiceCtrlHandler(tzServiceName,(LPHANDLER_FUNCTION)ServiceCtrlHandle);

	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{
		return;
	}

	//������������
	SetServiceStatus(hStatus,&ServiceStatus);

	//����һ���¼�����ͬ��
	hEventEnd = CreateEvent(NULL,TRUE,FALSE,NULL);
	if (hEventEnd == NULL)
	{
		return;
	}
	ResetEvent(hEventEnd);

	//�����������
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus,&ServiceStatus);

	//���������߳�
	hThread = CreateThread(NULL,0,ThreadMain,NULL,NULL,&dwThreadID);
	if (!hThread)
	{
		SetEvent(hEventEnd);
	}
	CloseHandle(hThread);

	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	SetServiceStatus(hStatus,&ServiceStatus);

	//�ȴ��¼��˳�,�������߳�
	WaitForSingleObject(hEventEnd,INFINITE);
	CloseHandle(hEventEnd);
}

void WINAPI ServiceCtrlHandle(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		//����moneyhub.exe
		if(g_terminateHandle != NULL)
			TerminateProcess(g_terminateHandle,0);
		//��ʼֹͣ
		ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hStatus,&ServiceStatus);
		//����ֹͣ�ź�
		SetEvent(hEventEnd);
		//ֹͣ
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus,&ServiceStatus);

		CloseHandle(hStatus);

		break;
	case SERVICE_CONTROL_PAUSE:
		ServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
		SetServiceStatus(hStatus,&ServiceStatus);
		ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		SetServiceStatus(hStatus,&ServiceStatus);
		break;
	case SERVICE_CONTROL_CONTINUE:
		ServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
		SetServiceStatus(hStatus,&ServiceStatus);
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(hStatus,&ServiceStatus);
		break;

	case SERVICE_CONTROL_INTERROGATE:	//��������״̬��ʱ
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		//����moneyhub.exe
		if(g_terminateHandle != NULL)
			TerminateProcess(g_terminateHandle,0);

		ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hStatus,&ServiceStatus);

		//����ֹͣ�ź�
		SetEvent(hEventEnd);

		break;
	default:
		ServiceStatus.dwCurrentState = dwOpcode;
		SetServiceStatus(hStatus,&ServiceStatus);
		break;

	}
}
/************************************************************************/
/*                          �����߳�                                    */
/************************************************************************/
DWORD WINAPI ThreadMain(LPVOID pParam)
{
	//WinExec("MoneyHub.exe -br",SW_HIDE);
	STARTUPINFOA ls;
	PROCESS_INFORMATION lp;
	ZeroMemory(&ls, sizeof(ls));
	ls.cb = sizeof(ls);
	ZeroMemory(&lp, sizeof(lp));
	
	CreateProcessA(NULL,"MoneyHub.exe -br",NULL,NULL,FALSE,0,NULL,NULL,&ls,&lp);
	g_terminateHandle=lp.hProcess;
	WaitForSingleObject(lp.hProcess,INFINITE);
	g_terminateHandle=NULL;

	return 0;
}