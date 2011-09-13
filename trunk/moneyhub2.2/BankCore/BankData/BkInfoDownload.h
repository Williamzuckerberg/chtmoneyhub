#pragma once

#include <map>
#include <string>
#include "DownloadManagerThread.h"
#include "../js/JSParam.h"
#include "../../BankData/BankData.h"




extern HWND g_hMainFrame;
class IBankDownInterface
{
public:
	IBankDownInterface(){};
	// bUpdate���ݰ汾�����ж��Ƿ��ٴ���ʾ, ur usbkeyr�������Ϣ�� bAddBkCtrl��ʾ�Ǵ������пؼ��ղ�
	virtual bool CheckServerXmlFile(bool bUpdate,  USBRECORD& ur) = 0;
	//���øýӿں󣬷���ture��ʾ��װ�ɹ���������Ҫ���°�װ
	virtual bool USBFinalTest(int vid, int pid, DWORD mid, bool& bSetup) = 0;
};
enum nsDownStates
{
	nsNULL,//��ʾ�ַ�Ϊ��
	nsDownloading,//��ʾ��������
	nsInstalling//��ʾ���ڰ�װ
};
// ������ʾ���ȵ���
class CNotifyFavProgress
{
public:
	CNotifyFavProgress();
	HWND m_hwndNotify;// ����ֻ���ҵ���ҳ���ͽ��ȣ�����ֻ����һ������͹���

	void SetFavProgress(string appId, int progress, nsDownStates dstate, bool allowState);
	//���ݿؼ����ȣ��ж���ʾ����
	void SetFavProgress(string appId, int progress);

	void CancelFav(string appId);//ȡ���ղ�

	void AddFav(string appId);// �����ղ�

	static CNotifyFavProgress*	m_Instance;

public:
	static CNotifyFavProgress* GetInstance();

	void SetProgressNotifyHwnd(HWND hNotify); // �������ؽ���֪ͨ�ľ��
};

class CBkInfoDownloadManager
{
public:
	~CBkInfoDownloadManager(void);

	// ��ȡ���ؽ���
	int ReadDownLoadPercent(LPSTR lpBankID);

	// �����Ƿ��Ѿ���װ��(��ʵ���Ǽ���������Ƿ��Ѿ����˸ÿؼ���)
	bool IsSetupAlready(LPSTR lpBankID);

	void CancleDownload(LPSTR lpBankID); // �û�ȡ������

	void PauseDownload(LPSTR lpBankID); // �û���ͣ����

	void FinishDLBreakFile(void);
	void CheckDownloadBreakFile(void); // ��������·���µ������ļ����Ƿ��������δ�������


	void MyBankCtrlDownload(LPSTR lpBankID); // �������пؼ�

	void MyBankUsbKeyDownload(LPUSBRECORD pUsbNode); // ����USBKEY // LPSTR lpBankID, LPVOID lpVoid, int nSize

	void SetCheckFun(IBankDownInterface* pObject);

	static CBkInfoDownloadManager* GetInstance(void);

	void SetProgressNotifyHwnd(HWND hNotify); // �������ؽ���֪ͨ�ľ��

	void NotifyCoreBankCtrlDLFinish(LPSTR lpBankID);

	static IBankDownInterface* GetInterfaceInstance(void); // �õ��ӿ�ָ��

protected:

private:
	// �������пؼ�(�������߳̽�������)
	void DownloadBankCtrl(LPSTR lpBankID,LPCTSTR lpszUrl = NULL, LPCTSTR lpszSaveFile = NULL, LPVOID lpPostData = NULL, DWORD dwPostDataLength = 0);
	CBkInfoDownloadManager();

	static DWORD WINAPI CheckThreadProc(LPVOID lpParam);
	
	bool ParseBkCtrlListContent(const char* pContent, std::list<std::wstring>& UrlList, std::string& strCheckCode); // �������пؼ�xml�ļ�����

	bool ParseUSBListContent(const char* pContent, std::list<std::wstring>& UrlList, std::string& strVersion, std::string& strCheck); // ����USBxml�ļ�����
	
	bool CheckBankCtrlXml(LPCTSTR lpPath, std::list<std::wstring>& UrlList, std::string& strCheckCode); // �������������ص�xml�ļ�

	bool CheckUsbKeyXml(LPCTSTR lpPath, std::list<std::wstring>& UrlList, std::string& strCheckCode, std::string& strVersion); // ��������USBKEYʱ���������ص�xml�ļ�

	// bool CheckUSBKeyXml(LPCTSTR lpPath, std::list<std::wstring>& UrlList, std::string& strCheckCode);

	bool ConstructBkCtrlCommunicateXml(LPSTR lpBankID, std::string& info); // ��������BankCtrl�ͷ�����ͨѶ��xml�ļ�

	bool ReadUSBCommunicateXml(const std::string& strIn, std::string& info); // ��ȡ����USB�ͷ���������ͨѶ��xml�ļ�

	bool CheckCheckCode(const std::string& strCheck); // ����У����

	std::string UrlEncode(const std::string& src); // �����ݽ��м���

	std::wstring GetTempCachePath(); // �õ�Ĭ�ϵ�����·��

	bool SetUpDownLoadFile(const std::wstring& strXmlFilePath, const std::wstring& strFilePath); // ��װ�Ѿ����غõ����

	void ReadAcquiesceSetupPath(); // ��ȡĬ�ϵİ�װ·��

	int MyTwoVersionCompare(std::string& strVer1, std::string& strVer2); // ���������汾�Ƚ�

	static DWORD WINAPI ShowUSBFinishThreadProc(LPVOID lpVoid);
	

	bool IsXmlBreakFileExistInPair(LPCTSTR lpPath, LPCTSTR lpFileName);// ����XML�ļ��Ͷϵ��ļ��Ƿ�һһ��Ӧ
	
	CDownloadManagerThread* FindBankCtrlDLManager(LPSTR lpBankID);

	std::map<std::string, CDownloadManagerThread*>	m_BankManager;
	std::wstring									m_wstrSetupPath; // ��װ·��
	std::wstring									m_wstrDLTempPath; // ���ص���ʱĿ¼ 
	std::wstring									m_strHWID;
	CRITICAL_SECTION								m_cs; // �ٽ���
	static IBankDownInterface*						m_staticpICheckFile;
	bool											m_bAddBkCtrl; // һ����־λ,������־���пؼ�ʱUSBKEY�������صģ������û�����ղ����ص�
	static CBkInfoDownloadManager*					m_staticInstance;
public:
	static DWORD WINAPI DownloadBkUSBThreadProc(LPVOID lpParam);
};