#pragma once
#include "../../Utils/CloudCheck/CloudCheckor.h"
#include "../../Utils/CloudCheck/CloudFileSelector.h"
#include "../../Utils/SecurityCache/SecuCache.h"
#include <string>
#include <set>
using namespace std;


#define MSG_BUF_LEN (256)

enum CheckStateEvent
{
	CHK_ERROR = -2,
	CHK_START = 0,
	CHK_SELFMODULES,
	CHK_SELFDATAFILES,
	CHK_SELFURLLIST,
	CHK_SELFSYSLIST,
	CHK_SYSLISTHASH,
	CHK_DRVIMAGEPATH,
	CHK_WINSXSDLL,
	CHK_CLOUDLISTHASH,
	CHK_SECURITYCHECK,
	CHK_END,
};

typedef void (*CHECKEVENTFUNC)(CheckStateEvent ev, DWORD dw, LPCTSTR lpszInfo, LPVOID lp);
// �ƽ�㰲ȫ����࣬������CProcessShow��Ϊ����ʾ����
class CSecurityCheck: public CProcessShow
{
public:
	CSecurityCheck();

public:
	// ������ȫ���ĺ���
	void Start(int bCheckType = 0);
	// ������ģ����м��ĺ���
	bool CheckSelfModules();
	// �����������ļ����м��ĺ���
	bool CheckSelfDataFiles(bool bCheckBank = true, bool bThreadCheck = false);
	// ���URL�������ļ��ĺ���
	bool CheckSelfUrlList();
	// ��ȡϵͳ��ѯ�ļ��ĺ���
	bool CheckSelfSysList();
	// �������״̬
	bool CheckDriverStatus();
	// �������gdi���󣬿��Ƿ����gdi�汾���ɵ�״��
	bool ExtraCheckGdiplus();

	int VerifyMoneyHubList(const char *path, const char* lpCHKFileName, wchar_t *message);

protected:
	// ��֤���пؼ�������
	bool  CheckBankDataFiles(LPSTR path,wchar_t * message);

	// ������֤�ļ���֤����ģ��ģ��
	int VerifySelfModules(const char* lpCHKFileName, wchar_t *message);
	// 
	int CheckMoneyHubList(unsigned char *buffer, int length, const wchar_t *path, wchar_t *message);
	
	int VerifyUrlList(const char* lpCHKFileName, wchar_t *message);
	int VerifySysList(const char* lpCHKFileName, wchar_t *message);
	// ��֤���ļ���ģ��
	int VerifyCloudList(const char* lpCHKFileName, wchar_t *message,CCloudFileSelector& cselector);
	// ����CProcessShow�ĺ���
	void Update(int i);
	// ��ȡ�Ʋ�ɱ�ļ������ݲ������ļ����ݵĺ���
	void SplitCloudListContent(const CStringA& strContent,CCloudFileSelector& cselector);
	// ��ȡϵͳ�ļ��б��ļ��ĺ���
	void ReadSysList(const CStringA& strContent);




public:
	void SetEventsFunc(CHECKEVENTFUNC pfn, LPVOID lpData);

protected:
	// ��ȫ��ⴰ�ڷ��ͼ������¼��ĺ���
	void Event(CheckStateEvent cse, DWORD prog_or_error, const wchar_t* lpszInfo, const char* lpszErrNum);
	// ��������2���ӵļ��
	static DWORD WINAPI _threadCheck(LPVOID lp);
	// ��װʱ���а�ȫ�����߳�
	static DWORD WINAPI _threadInstallCheck(LPVOID lp);
	// ���л�������߳�
	static DWORD WINAPI _threadCacheCheck(LPVOID lp);

private:
	set<wstring> m_files;
	CHECKEVENTFUNC m_fnCheckEventFunc;
	LPVOID m_lpData;
	// ����2���Ǹ������а�ȫ�����ʾ���ȵļ�¼����
	static int ProcessPos;
	static int ProcessHelp;//��������

	// ��¼��Ҫ���м���������
	int m_listnumber;
	// ��¼��⵽������
	int m_nowcheck;
	// �����ڼ����ͬ��
	CRITICAL_SECTION m_cs;
public:
	bool CheckWhiteListCache();// ���ɰ�����
	bool CheckBlackListCache();// ���ɺ�����
	bool CloudCheck(set<wstring>* files,bool& flag);
	bool CheckCache();
};

extern CSecurityCheck _SecuCheck;