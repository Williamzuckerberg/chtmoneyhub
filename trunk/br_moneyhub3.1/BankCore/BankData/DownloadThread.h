#pragma once


enum {
	ERR_STOPPED = -1,
	ERR_SUCCESS = 0,
	ERR_URLCRACKERROR = 1,
	ERR_NETWORKERROR = 2,  // �������
	ERR_FILENOTFOUND = 3, 
	ERR_DISKERROR = 4, 
	ERR_FATALERROR = 5,
//	ERR_OUTOFTIME = 6, 
	ERR_UNKNOW = 7,
};


enum MY_DLTHRED_STATE
	{
	emInit = -1,
	emBegin = 0,
	emFinished = 1,
	emCancled = 2,
	};

enum MY_DOWNLOAD_KIND
{
	emOther = 0x00000000,
	emBkActiveX = 0x00000001,
	emUsbKey = 0x00000010,
	emUSBBkActiveX = 0x00000011, // ��USBKEY����ʱ��ֻ�������пؼ���������USBKEY����
};

enum MY_DOWNLOAD_ERR
{
	emUsbKeyDlErr,
	emUsbKeySetupErr,
	emBankCtrlDlErr,
	emBankCtrlSetupErr,
};

typedef struct _DWON_LOAD_USBKEY
{
	_DWON_LOAD_USBKEY ()
	{
	};

	int nVid; // 
	int nPid;
	int nMid;
	std::string strVersion; // �������صİ汾

}DWON_LOAD_USBKEY, *PDWON_LOAD_USBKEY;

// ���ز����ṹ��
typedef struct _DOWN_LOAD_PARAM_NODE
{
	_DOWN_LOAD_PARAM_NODE ()
	{
		bSetupDlFile = false;
		emKind = emOther;
	};

	std::wstring strHWID; // ����������Ӳ��ID
	std::wstring strUrl; // �ļ����ص�URL����
	std::wstring strSaveFile; // �ļ�����ʱ�����·��
	std::wstring strSetupPath; // �ļ����غ�װ��·��
	std::string  strSendData;
	//LPVOID lpPostData; // ���͵��������˵�����
	//DWORD dwPostDataLength; // ���͵����ݿ�˵ĳ���
	bool bSetupDlFile; // �Ƿ����غ��˵��ļ����а�װ
	MY_DOWNLOAD_KIND emKind; // ���ص�����
	bool bCreateThread; // �Ƿ񴴽�һ���µ��߳�

	DWON_LOAD_USBKEY dwUsbKeyParam;

}DOWN_LOAD_PARAM_NODE, *PDOWN_LOAD_PARAM_NODE;

// �ļ�������
class CDownloadThread
{
public:
	CDownloadThread();
	~CDownloadThread();

	void CancleDownload(void); // �û�ȡ������
	//void PauseDownload(void); // �û���ͣ����
	
	void ReadDownloadPercent(UINT64& i64FileSize, UINT64& i64ReadSize); // ��ȡ���ļ����صĽ���

	int GetDownloadPercent(void);

	LPCTSTR GetDownloadURL(void);

	LPCTSTR GetDownloadStorePath(void);

	LPCTSTR GetBreakFilePath(void);


	// ��������·��ת���ɶϵ��ļ�·��
	static bool TranslanteToBreakDownloadName(const std::wstring& strPath, std::wstring& strDesc);
	// ���ϵ��ļ�·��ת����������·��
	//static bool TranslanteToOriginalFileName(const std::wstring& strPath, std::wstring& strDesc);

	void DownLoadInit(LPCTSTR lpszHWID, LPCTSTR lpszUrl, LPCTSTR lpszSaveFile, LPSTR lpSendData);

	//void BeginDownloadByThread(bool bCreateThread = true); // �Ƿ������߳�ʽ��ʼ����
	int DownLoadData();

protected:
	bool IsCancled(void);

	int GetThreadState(void);
	int TransferDataGet();
	int TransferDataPost();
	//LPCTSTR GetSavePath(void);

private:
	void CloseHandles();
	void SetupDownloadFile(); // ��װ�ļ�
	UINT64  IsBreakPointFile(std::wstring wcsFile);
	int DownLoadBreakpointFile();

	//static DWORD WINAPI ThreadProc(LPVOID lpParam); // �����߳�ʽʵ��

private:
	
	std::wstring	m_strSaveFile; // �ļ������·��
	std::wstring	m_strDownURl; // �����ļ���URL
	std::wstring	m_strHWID; // ���ؼ������Ӳ��ID
	std::string		m_strSendData; // Ҫ���͵�������������

	bool			m_bCancle; // �Ƿ��û�ȡ��
	int				m_bDLThreadState; // �������ص�״̬
	
	DWORD			m_dwPostDataLength; // Ҫ���͵����������ݵĳ���

	HINTERNET		m_hInetSession; // �Ự���
	HINTERNET		m_hInetConnection; // ���Ӿ��
	HINTERNET		m_hInetFile; //
	HANDLE			m_hSaveFile;

	std::wstring	m_wcsBreakFileName; // ת�������ʱ�ļ�ȫ·��
	UINT64			m_ui64FileSize; // Ҫ���ص��ļ���С
	UINT64			m_ui64TotalRead; // �ܹ����صĴ�С
	UINT			m_repeatNum; // ���ӷ�����ʧ�ܵ��ܴ���

	//CRITICAL_SECTION m_cs; // ����ȡ������ͣʱ������ͬ��ʱ��
	bool			m_bRetryWait; //  ����״̬
	bool			m_bCreateThread;

};

class CDownloadAndSetupThread : public CDownloadThread
{
public:
	CDownloadAndSetupThread();
	~CDownloadAndSetupThread();

	void DownLoadAndSetupDlFile(PDOWN_LOAD_PARAM_NODE pDownloadParam);

	MY_DOWNLOAD_KIND GetDownloadKind (void); // �õ����ؿؼ�����

	bool IsFinished(void);

	bool IsCancled(void);

	void CancleDownloadAndSetup(void);

public:
	DWON_LOAD_USBKEY m_dlUSBParam;

private:

	static DWORD WINAPI DLAndSetupThreadProc(LPVOID lpParam); // ���غͰ�װ�߳�ʽʵ��
	bool SetupDownloadFile(MY_DOWNLOAD_KIND emKind,LPCTSTR lpPath, LPCTSTR lpSetUp); // ���а�װ
	bool SetupBankControl(LPCTSTR lpPath, LPCTSTR lpSetUp); // ���пؼ���װ
	bool SetupBankUsbKey(LPCTSTR lpPath, LPCTSTR lpSetUp); // ����USBKEY��װ
	bool CheckDownLoadFile(LPCTSTR lpPath); // У�����ص��ļ�
	bool CheckDlFileAndShowErrMeg(LPCTSTR lpPath, MY_DOWNLOAD_KIND emKind);
	void SetupDlFileOrNotNeed(MY_DOWNLOAD_KIND emKind, bool bSetup);// ���Ҫ��װ��ִ�а�װ��������ð�װ������

private:
	bool				m_bSetupDlFile; // �Ƿ����غ��˵��ļ����а�װ
	bool				m_bSetupFinish; // ��������Ƿ�װ���
	bool				m_bCancled;
	bool				m_bThreadExcute;
	MY_DOWNLOAD_KIND	m_emDlKind;
	std::wstring		m_strDlPath;
	std::wstring		m_strSetupPath;
	//CRITICAL_SECTION	m_cs; // ����ȡ������ͣʱ������ͬ��ʱ��
};
