#pragma once

#include <string>
#include <vector>

#define  BANKINFOPATH        L"BankInfo\\banks\\*.*"
#define  BANKINFOPATHNOFIND  L"BankInfo\\banks\\"
#define  XMLMODULEINFO       "<module><name>%s</name><version>%s</version></module>"

typedef enum 
{
	BLACKUPDATE =0,
	WHITEUPDATE,
	ALL,
	NONE

}WBRETURN;

class CDownloadOperation
{
public:
	CDownloadOperation(void);
	~CDownloadOperation(void);
private:
	/**
	*   all path 
	*/
	std::wstring getModulePath();
	std::wstring getAppDataPath();
	/*
	bChoose = true;		%temp%
	bChoose = false;    %appdata%
	*/
	std::wstring  getBankCachePath(bool bChoose = false);


	void  deleteDirectoryW(LPWSTR path);
	void  uncompressTraverse(LPWSTR path , BYTE bIndex,LPWSTR parentDirectory = NULL);

	/**
	*��֤���������ļ��Ƿ�����ڰ��������棬��Ч�黺���ļ��Ƿ��б仯�������û��ȴ�ʱ��
	*/
	WBRETURN IsNewTBCacheFile(wchar_t * pFileName);
	bool     UpdateWBFile(WBRETURN wb);
public:
	/**
	*��ѹ������ģ���ļ�����װĿ¼
	*/
	void   uncompressFile();
	/**
	*ɾ�����ص������ļ�
	*/
	void deleteAllUpdataFile(wchar_t* wcsPath = NULL);

	bool   getXMLInfo(LPSTR content,DWORD dwlen,LPSTR version,LPSTR moduleName = NULL);

	/**
	*   ������غ���ļ��ĺϷ��ԣ��类�ƻ�����ɾ��  . return : 0 == ȫ�����ϸ�,1 ==���ֺϸ� ,2 ==ȫ���ϸ�
	*/
	int isValid();

	bool isSpecialCab(wchar_t * wcsP);

	void ShowMessage();

	/**
	*  ��������ǰɾ����װĿ¼�����������ļ������浽��ʱ�ļ����ڣ��Ա�����ʧ�ܺ�ԭ
	*/
	void moveFileTraverse(IN LPWSTR path, IN  LPWSTR wcsExsitingFile, IN  LPWSTR wcsNewFile);
	void moveFiles(LPWSTR wcsFileName, bool bDirection);
private:
	std::vector <std::wstring> m_vecSpecialDName;
	std::vector <std::wstring> m_vecWBFileName;
};
