#pragma once
#include "../stdafx.h"
#include <map>
#include <list>
#include <string>
using namespace std;
#include "../../BankData/BankData.h"

enum BillState
{
	bSNormal,
	bSExceedTime,
	bSCancel
};
typedef struct urllist
{
	int type;//����
	map<int, wstring> url;//�ؼ������url����
	vector<int> m_beginstep;//��¼��ʼ����Ŀ�ʼ 
}URLLIST, *LPURLLIST;

typedef struct BillUrlData
{
	string			id;			// �������
	wstring			name;		// ��������
	wstring			dll;		// ��̬������
	int             m_islogin;   //login
	int             m_mode;       //mode
	list<URLLIST>	urldata;
}BILLURLDATA;

// �����˵���¼����
class CBillUrlManager
{
private:
	CBillUrlManager();
	~CBillUrlManager();

	static CBillUrlManager* m_Instance;
public:
	static CBillUrlManager* GetInstance();
	bool Init();

	// �洢�����˵����õ�dll���
	HMODULE					m_urldll;

private:
	// �洢�����˵����������õ�����ַ������
	list<BILLURLDATA>		m_billlist;
	// ��ȡ�����˵���ַ�����ļ�
	std::string GetFileContent(wstring strPath,bool bCHK);

	// �洢�͵����˵���������̺߳����ݣ���֤�ɵ����˵����̴߳��������߳�Ҳ���ҵ�Ҫ�����˵�����������������
	map<DWORD, BillData*>	m_billTid;

	BILLRECORDS			m_BillRecords;	//�˵���¼��ָ�룬��Ҫ��֤���߳�ʹ��ʱ��˳��ִ�в��������

	CRITICAL_SECTION		m_cs; // �ٽ���,��ʱû��
public:	
	// ���ݻ���aid���˵����ͻ��������Ӧ��url�͹ؼ���������ݶ�Ӧ��ϵ
	LPBILLRECORDS GetBillRecords()
	{
		return &m_BillRecords;
	}
	LPURLLIST  GetUrlMap(string aid, int type);
	wstring GetBillUrl(string aid, int type, int step, bool& isBeginStep);

	bool InsertBillTid(DWORD pid, BillData* pData);
	BillData* GetBillTid(DWORD pid);
public:
	void ShowResultDlg();
	wstring GetDllName(string aid);
	int  Getislogin(string aid);
	int  Getmode(string aid);
	bool DeleteBillTid(DWORD pid);
	void FreeDll();
	//CComVariant CallJScript(IHTMLDocument2 *doc, std::string strFunc,std::vector<std::string>& paramVec);
	//CComVariant CallJScript2(IHTMLDocument2 *doc, std::string strFunc, DISPPARAMS& dispparams);
	// ���˵�����д�����ݿ�Ľӿ�
	void SendBillRecordToJS(BillData* pData);
public:
	// ����dll������ҳ���ݺͲ������ݵĽӿ�
	int GetBill(IWebBrowser2* pFatherWebBrowser, IWebBrowser2* pChildWebBrowser, BillData* pData, int& step, HWND hAxControl = NULL);

	void SetGetBillState(BillState bState);
	void SetNotifyWnd(HWND notifyWnd);
	void InitDll();
	void FilterXml(char* pString, int maxlength = 256);
};