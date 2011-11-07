#pragma once
#include "../../Utils/SecurityCache/SecuCache.h"
#include <string>
#include <set>
#include <map>
#include <Message.h>
using namespace std;



class CGlobalData
{
private:
	CSecurityCache m_blackcache; // ������
	CSecurityCache m_whitecache; // ������

	set<wstring> m_graylist;	//�Ʋ�ɱ�����б�
	set<wstring> m_waitlist;    // ����ɱ�б�
	map<wstring,wstring> m_LogicToDosDevice;//��¼���������̷���Ӧ��ϵ
	HANDLE m_hProcessMutex;
	HWND  m_frame;
public:
	void Init();
	void Uninit();
private:

	CGlobalData(void);
	~CGlobalData(void);

	static CGlobalData* m_Instance;
public:
	static CGlobalData* GetInstance();

public:
	void SetFramHwnd(HWND fhwnd);
	map<wstring,wstring>* GetLogicDosDeviceMap();
	set<wstring>* GetWaitList();// �Ʋ�ɱ�����б�Ϊδ����΢����֤��Ҫ�����Ʋ�ɱ���ļ�
	set<wstring>* GetGrayList();// ��Ҫ�����ļ�����Ҫ�Ǵ�������ȡ�Ļ��ļ�
	CSecurityCache* GetBlackCache();
	CSecurityCache* GetWhiteCache();

	BOOL CloseMoneyHub();// �ر��������е�moneyhub�Ƿ���������
	BOOL IsPopAlreadyRunning(); // ���Pop�Ƿ���������

	void ShowCloudMessage();
	void ClearCloudMessage();
	void ShowCloudStatus();
	void NoShowCloudStatus();

//�µİ�ȫ����
private:
	std::set<std::wstring>  m_unKnowFileList;
	std::set<std::wstring>  m_whiteFileList;
	std::set<std::wstring>  m_blackFileList;
public:
	set<wstring> * GetBlackFileList();
	set<wstring> * GetWhiteFileList();
	set<wstring> * GetUnKnowFileList();
};