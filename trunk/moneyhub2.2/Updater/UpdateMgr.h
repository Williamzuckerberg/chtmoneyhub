#pragma once

#include <string>
#include <vector>
#include "../Security/Authentication/BankMdrVerifier/export.h"
#include "..//Utils/CryptHash/base64.h"

enum UpdateMgrEvent
{
	UE_EVERYTHING_OK = 0,

	UE_PROGRESS_VALUE,

	UE_RETR_LIST,	
	UE_RETR_FILE,
	UE_RETR_SIGN,
		
	UE_DONE_LIST,				
	UE_DONE_FILE,
	UE_DONE_SIGN,

	UE_CHECK_LIST,
	UE_CHECK_LIST_FAILED,

	UE_AVAIL_YES,		
	UE_AVAIL_NO,		

	UE_CHECK_FILE,
	UE_CHECK_FILE_FAILED,

	UE_INET_ERROR,
	UE_FILE_ERROR,
	UE_FATAL_ERROR,					
};

typedef void (*UPDATEMGREVENTFUNC)(UpdateMgrEvent ev, LPCTSTR info, LPVOID lp);

class CUpdateMgr
{
	friend class CDownloadOperation;
	friend class CHttpDownloader;
	friend class CMainDlg;
public:
	CUpdateMgr();
	~CUpdateMgr();

public:
	void Start(tstring strUrl, LPVOID lpPostData, DWORD dwPostDataLength);
	void Stop();
	bool IsRunning() const;
	int checkFileValid();//��װǰ��֤�ļ��Ϸ��� . 0 == ȫ�����ϸ�, 1 == ���ֺϸ�, 2==ȫ���ϸ�
	bool isSafeFile(wchar_t * pWcsFileName);//�жϸ��ļ��Ƿ���ͨ����֤�ļ�
public:
	void SetEventsFunc(UPDATEMGREVENTFUNC func, LPVOID lpVoid);
	void Event(UpdateMgrEvent ume, LPCTSTR info = NULL);

public:
	UINT64 GetBytesCount() const { return m_ui64FileSize; }
	UINT64 GetBytesRead() const { return m_ui64FileRead; }

protected:
	bool GetUpdateList();		// ����LIST
	bool CheckListFile();		// ���LIST�ļ�
	bool GetUpdateFiles();		// �������ݰ�
	bool GetSignatureFiles();	// ����ǩ��
	int  CheckUpdateFiles();	// ������ݰ�  0 == ��֤��Ч��1 == �����ļ���֤�ϸ� ��2== �ļ�ȫ����֤�ϸ� 
protected:
	static DWORD WINAPI _threadUpdate(LPVOID lp);
	void ErrCode2Event(int nErrCode);
	void SetProgressVal(UINT64 uSize, UINT64 uRead);
	bool ParseListContent(LPBYTE pContent, DWORD dwSize);

	std::wstring getDownLoadFilePath();
	bool getLocalFilePathName();
	
protected:
	UINT64 m_ui64FileSize;
	UINT64 m_ui64FileRead;

	DWORD m_dwStatusId;
	tstring m_strStatusMsg;

	bool m_bCriticalPack;

	tstring m_strListUrl;
	std::vector<tstring> m_vecFileUrls;
	std::vector<tstring> m_vecSignUrls;

	tstring m_strLocalList;
	std::vector<tstring> m_vecLocalFiles;
	std::vector<tstring> m_vecLocalSigns;

	std::vector<std::string>  m_checkBase64;//base64 ��֤��
	std::vector<std::string>  m_vecUpgradeContent;//��������
	bool m_bRunning;
	bool m_bNeedStop;

	LPVOID m_lpPostData;
	DWORD m_dwPostDataLength;

	UPDATEMGREVENTFUNC m_funcEvent;
	LPVOID m_lpParam;

	static  bool m_isMain;

	//ͨ����֤�ļ�����
	std::vector <std::wstring> m_vecPassCheck;
};

extern CUpdateMgr _UpdateMgr;
