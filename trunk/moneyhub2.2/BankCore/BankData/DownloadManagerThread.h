#pragma once

#include <list>
#include <map>
#include "DownloadThread.h"

class CDownloadManagerThread
{
	
public:
	CDownloadManagerThread();
	~CDownloadManagerThread();

	// ���һ����������
	//bool CreateDownLoadTask(LPCTSTR lpszHWID, LPCTSTR lpszUrl, LPCTSTR lpszSaveFile, LPVOID lpPostData = NULL, DWORD dwPostDataLength = 0, bool bCreateThread = true);
	bool CreateDownLoadTask(PDOWN_LOAD_PARAM_NODE pNode);
	void CancleAllDownload(); // �û�ȡ������
	void PauseAllDownload(); // �û���ͣ����
//	void ContinueDownload(LPCTSTR lpszUrl); // ��������
	int GetAverageDownLoadProcess(); // �õ����ؽ���(-1����ʧ�ܣ�0-100֮���ʾ���ȣ�200����װ��300��ʾ���)
	
	int GetDownLoadThreadSize(); // �õ������̵߳ĸ���

//	bool AllDownloadFinished(void); // �����Ƿ����Ѿ���ɵ��߳�

	bool AllDownloadThreadExit(void); // �������е��߳��Ƿ��Ѿ��˳�

	bool HasDownloadTask(void); // �Ƿ�������ص�������

	CDownloadAndSetupThread* ReadAFinishDownloadThread();

	void ReadAllDownLoadPath(std::list<std::wstring>& listStor);

	void DeleteAllDownLoadThread(void); // ɾ�������߳�

	//bool IsReadable(void); // �Ƿ��ܶ�

	bool IsBankCtrlSetup(void); // �����е����пؼ��Ƿ��Ѿ���װ

	bool IsBankCtrlCancled(void); // ���пؼ�ȡ���ղ�

	//void SetReadState(bool bRead = true);

	bool AddBankCtrlAlready(void);

	// void TagBankCtrlSetup(void); // �������б�ǳ��Ѿ���װ
private:
	CDownloadAndSetupThread* GetDLThread(LPCTSTR lpszUrl);
	
private:
	LPCRITICAL_SECTION	m_lpCS;
	std::map<CDownloadAndSetupThread*, bool> m_pDLThreadList; // bool����������ʾ��Ӧ�������߳���ɺ��Ƿ��Ѿ�������̶߳�ȡ��
};