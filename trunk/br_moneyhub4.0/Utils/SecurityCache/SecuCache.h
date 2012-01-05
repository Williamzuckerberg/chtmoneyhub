#pragma once
#define _MD5

#include <list>
#include <string>
using namespace std;
//////////////////////////////////////////////////////////////////////////
#ifdef _MD5
#define		SECURE_SIZE	16
#endif
#include "windows.h"
#include "../../Security/BankLoader/BankLoader.h"


#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define		SECURE_BUCKET_SIZE		256
struct SecCachStruct
{
	wchar_t filename[MAX_PATH];
	unsigned char chkdata[SECURE_SIZE];
	short int tag;//0������Cache�ļ��ж�����1����������2�����и��£�3��������,1\3Ϊ��Ч���ݣ�0,2Ϊ��Ч����,4����Ϊѧϰ������
};

class CSecurityCache
{
public:
	CSecurityCache();
	virtual ~CSecurityCache();
private:
	list<SecCachStruct*>  m_secuBucket[SECURE_BUCKET_SIZE];

	wstring m_cathfile;
	unsigned char *evalueCache;
	int m_length;

	bool m_isChange;
	bool m_isShouldSend;
	int m_number;
public:
	bool Init();
	int	 GetFileNumber();
	bool Add(SecCachStruct& sec,int style = 0);//�����ѧϰ�ģ�style����Ϊ1��������Ĭ�ϵ�
	bool IsInSecurityCache(SecCachStruct& sec);
	bool SetCacheFileName(wchar_t* secname);
	bool Clear();
	bool Flush();
	bool SetAllDataInvalid();//����ԭ�����е����ݶ�����Ч�ģ��ڵ������ӵ�ʱ�����á�
	

	bool CalculEigenvalue(SecCachStruct& sec);
private:
	void CheckUpdate(int& effectnum);
	unsigned char HashFunc(wchar_t *filename);

public:
	bool IsMZFile(const wchar_t* pfile);
	bool IsChanged();
	bool IsShouldSend();
	bool SetSend(bool isSend);
	bool GetEigenvalue(unsigned char *data,DWORD& length);
	bool GetEigenvalue(ModuleList& mlist);
};
