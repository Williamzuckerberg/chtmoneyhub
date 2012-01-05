#pragma once
#include <set>
#include <string>
#include "Shlwapi.h"
#include <windows.h>
#pragma comment(lib, "Advapi32.lib")
using namespace std;


class CCloudFileSelector
{
public:
	CCloudFileSelector(void);
	~CCloudFileSelector(void);
private:
	set<wstring> m_cloudfile;

	wstring m_java;
	int m_sysversion;


	//wchar_t m_cloudcheckfile[MAX_PATH];
	
public:
	void AddWhiteList(int tag,wstring filename);
	void AddFolder(wstring folder);

	void GetAllFiles();
	set<wstring>* GetFiles();
	void ClearFiles();

	void AddRegFolder(wstring regflag,wstring keyname,int flag);
	void AddRegFile(wstring regflag,wstring key,wstring file,int flag);
	void AddExtensionsFile(wstring folder,wstring extensions);
private:
	//���Ҫ���ҵ�Java�ļ�
	bool GetJavaInstallDirectory();
	//������뷨�ļ�
	bool GetInputFiles();
	//��ô�ӡ�������ļ�
	bool GetPrintDriver();
	//�������������ֱ��ò�ͬ��ӡ�������Ĳ���
	bool GetPrintEnvironments();
	bool GetPrintMonitors();
	bool GetPrintProviders();
	bool GetValue(wchar_t* para,wchar_t* value);
	bool GetStringFiles(const wchar_t* str,int len,const wstring& dir);

	//���Win7�ļ�
	bool GetWinVer();

	bool GetIEFiles();


	//������������ļ�
	bool GetThemeFiles();

	bool GetSharedDLLs();
	void EnumDir(wstring resToken);
};
