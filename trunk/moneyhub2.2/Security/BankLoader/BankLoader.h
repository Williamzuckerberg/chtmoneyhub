// BankLoader.h
#pragma once 
#include "DriverManager.h"
#include "atlstr.h"

class CBankLoader;

#include <vector>

struct LoaderInfo
{
	CBankLoader* bankLoader;
	CString cmdline;
};

struct ModuleItem
{
	CStringW path;
	bool bVerified;
	unsigned char md[20]; //zjun
};

typedef std::vector<CStringW> SysModuleList;
extern SysModuleList g_sysModuleNameList;

typedef std::vector<ModuleItem> ModuleList;
extern ModuleList g_moduleHashList;
extern ModuleList g_blackHashList;

extern DWORD g_dwProcessId;

//look up dll
//extern  UINT32 g_kernelProcessId;
extern  unsigned char g_kernelHash[400000] ;

class CBankLoader
{
	static DWORD WINAPI LoadProc_S(LPVOID param);
	DWORD LoadProc(LoaderInfo* loaderinfo);

	//get lsass.exe PID
	bool returnLsassId(wchar_t *p, unsigned long * pId);
	bool getPriviledge();
public:
	static CString TranslateName(const WCHAR* );
	static CStringW GetDevName(const WCHAR* wcsFileName);
	static CString GetFileName(const WCHAR* );

private:
	bool m_bLoadFinished;

	CBankLoader();
	~CBankLoader();

	static CBankLoader* m_Instance;
public:
	CDriverManager m_DriverLoader;
	static CBankLoader* GetInstance();

	bool InstallAndStartDriver();
	bool UnInstallDriver();

	bool LoadProcess(const CString& exepath, HANDLE& hProcess, DWORD& PID);
	bool IsFinished();

	bool Safeguard();
	bool setSecuModHashBR();

	bool SendBlackHashListToDriver();

	bool SendReferenceEvent(HANDLE& ev);//evΪ���������Ϊ����֪ͨӦ�ó����л������ļ�

	bool GetGrayFile(void *pFileData, DWORD size);//pFileDataΪ������������������ݷ�������ݶΣ���ʽΪ����������(DWORD) + �ļ�1��(MAX_PATH(260)*sizeof(WCHAR)) + md5ֵ( 16 ) + �ļ�2�� + ...,sizeΪ�����ݶγ���

	bool CheckDriver();//�������״̬�ĺ����������2����
};