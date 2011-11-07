#include "stdafx.h"
#include "Security.h"
#include "../stdafx.h"

CGlobalData* CGlobalData::m_Instance = NULL;

CGlobalData* CGlobalData::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CGlobalData();
	return m_Instance;
}

CGlobalData::CGlobalData(void):m_hProcessMutex(0),m_frame(0)
{
}
CGlobalData::~CGlobalData(void)
{
}

void CGlobalData::Init()
{
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_INIT, L"��ʼ���ڰ�����");

	wstring drvName[26] = {L"A:",L"B:",L"C:",L"D:",L"E:",L"F:",L"G:",L"H:",L"I:",L"J:",L"K:",L"L:",L"M:",L"N:",L"O:",
		L"P:",L"Q:",L"R:",L"S:",L"T:",L"U:",L"V:",L"W:",L"X:",L"Y:",L"Z:"};
	WCHAR devName[MAX_PATH];
	for(int i = 0;i< 26; i++)
	{
		// ���ϵͳ�������߼����̵�Dos����Ŀ¼����ת��
		if(QueryDosDeviceW(drvName[i].c_str(), devName, MAX_PATH) > 0)
			m_LogicToDosDevice.insert(std::make_pair(drvName[i], devName));
	}

	m_whitecache.SetCacheFileName(L"%Appdata%\\Moneyhub\\WhiteCache.dat");// ��ʼ��������
	m_whitecache.Init();
	m_blackcache.SetCacheFileName(L"%Appdata%\\Moneyhub\\BlackCache.dat");// ��ʼ��������
	m_blackcache.Init();
	m_waitlist.clear();
	m_graylist.clear();
}
void CGlobalData::Uninit()
{
	m_whitecache.Clear();
	m_blackcache.Clear();
	m_waitlist.clear();
	m_graylist.clear();
}
map<wstring,wstring>* CGlobalData::GetLogicDosDeviceMap()
{
	return &m_LogicToDosDevice;
}

set<wstring>* CGlobalData::GetWaitList()//�������Ʋ�ɱ�����б�
{
	return &m_waitlist;
}
CSecurityCache* CGlobalData::GetBlackCache()
{
	return &m_blackcache;
}
CSecurityCache* CGlobalData::GetWhiteCache()
{
	return &m_whitecache;
}
set<wstring>* CGlobalData::GetGrayList()// �ܵĲ�ɱ�б�
{
	return &m_graylist;
}
void CGlobalData::ShowCloudMessage()
{
	HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
	if (hWnd)
	{
		PostMessage(hWnd, WM_CLOUDALARM, 0, 0);//
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_UI_COM, L"ShowCloudMessage");
	}
}
void CGlobalData::ClearCloudMessage()
{
	HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
	if (hWnd)
	{
		PostMessage(hWnd, WM_CLOUDCLEAR, 0, 0);//
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_UI_COM, L"ClearCloudMessage");
	}
}
void CGlobalData::ShowCloudStatus()
{
	HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
	if (hWnd)
	{
		PostMessage(hWnd, WM_CLOUDCHECK, 0, 0);//
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_UI_COM, L"ShowCloudStatus");
	}

}
void CGlobalData::NoShowCloudStatus()
{
	HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
	if (hWnd)
	{
		PostMessage(hWnd, WM_CLOUDNCHECK, 0, 0);//
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_UI_COM, L"ShowCloudStatus");
	}
}
// �ر�MoneyHub����
BOOL CGlobalData::CloseMoneyHub()
{
	LPCTSTR lpszProcessMutex = _T("_Moneyhub_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B6}");
	HANDLE _hProcessMutex  = CreateMutex(NULL, TRUE, lpszProcessMutex);

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_UI_COM, L"CloseMoneyHub");
	DWORD err = GetLastError();
	if(_hProcessMutex != NULL)
		CloseHandle(_hProcessMutex);
	// ���ں˶����Ѿ����ڻ��߽�ֹ����ʱ��˵���Ѿ������ý���
	if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
	{
		int time = 0;
		//������
		while( 1 )
		{
			if(time > 5)
				break;
			time ++;
			HWND hWnd = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
			if (hWnd)
			{
				::SendMessageTimeoutW(hWnd, WM_CLOUDNDESTORY, 0, (LPARAM)m_frame, SMTO_NORMAL, 2000, 0);//
				return TRUE;
			}
			else
				Sleep(2000);
		}
	}
	return FALSE;
}

BOOL CGlobalData::IsPopAlreadyRunning()
{
	LPCTSTR lpszProcessMutex = _T("_MoneyhubPop_{878B413D-D8FF-49e7-808D-9A9E6DDCF2B9}");
	HANDLE _hProcessMutex  = CreateMutex(NULL, TRUE, lpszProcessMutex);

	DWORD err = GetLastError();
	// ���ں˶����Ѿ����ڻ��߽�ֹ����ʱ��˵���Ѿ������ý���
	if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
	{
		return TRUE;
	}
	return FALSE;
}

void CGlobalData::SetFramHwnd(HWND fhwnd)
{
	m_frame = fhwnd;
}


////////////////////////////////////////////
set<wstring> * CGlobalData::GetBlackFileList()
{
	return &m_blackFileList;
}

set<wstring> * CGlobalData::GetWhiteFileList()
{
	return &m_whiteFileList;
}

set<wstring> * CGlobalData::GetUnKnowFileList()
{
	return &m_unKnowFileList;
}