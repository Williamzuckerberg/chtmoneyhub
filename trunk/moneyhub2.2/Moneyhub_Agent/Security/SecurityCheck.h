#pragma once
#include "../../Utils/CloudCheck/CloudCheckor.h"
#include "../../Utils/CloudCheck/CloudFileSelector.h"
#include "../../Utils/SecurityCache/SecuCache.h"
#include <string>
#include <set>
using namespace std;


#define MSG_BUF_LEN (256)

// �����ݽ����У�ֻ����뻺��ͺڰ������йصģ���������һ������
// �ƽ�㰲ȫ����࣬������CProcessShow��Ϊ����ʾ����
class CSecurityCheck
{
public:
	CSecurityCheck();
	~CSecurityCheck();

public:
	// ������ȫ���ĺ���
	void Start(int bCheckType = 0);

	// ��ȡϵͳ��ѯ�ļ��ĺ���
	bool CheckSelfSysList();
	// �������״̬
	//bool CheckDriverStatus();

protected:
	int VerifySysList(const char* lpCHKFileName, wchar_t *message);
	// ��֤���ļ���ģ��
	int VerifyCloudList(const char* lpCHKFileName, wchar_t *message,CCloudFileSelector& cselector);
	// ��ȡ�Ʋ�ɱ�ļ������ݲ������ļ����ݵĺ���
	void SplitCloudListContent(const CStringA& strContent,CCloudFileSelector& cselector);
	// ��ȡϵͳ�ļ��б��ļ��ĺ���
	void ReadSysList(const CStringA& strContent);

protected:
	// ��������2���ӵļ��
	static DWORD WINAPI _threadCheckAuto(LPVOID lp);
	// ��װʱ���а�ȫ�����߳�
	static DWORD WINAPI _threadSelfCommunicate(LPVOID lp);
	// ��鰲ȫ������߳�
	static DWORD WINAPI _threadCacheCheck(LPVOID lp);

	// ��ñ��ι�������
	//bool GetFilterFile(list<wstring>& filterfiles);


private:
	set<wstring> m_files;
	LPVOID m_lpData;
	// ��¼��Ҫ���м���������
	int m_listnumber;
	// ��¼��⵽������
	int m_nowcheck;
	// �����ڼ����ͬ��
	CRITICAL_SECTION m_cs;
public:
	bool CheckSercurityCache(bool& isCacheExist);//��ⰲȫ����״̬
	bool CloudCheck(set<wstring>* files,bool& flag);
	bool CheckCache();
	bool CheckBlackListCache();
	bool ReBuildSercurityCache();
};

extern CSecurityCheck _SecuCheckPop;