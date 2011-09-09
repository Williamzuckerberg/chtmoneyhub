#pragma once
#include "../stdafx.h"
#include <map>
#include <list>
#include <string>
using namespace std;
#include "../../BankData/BankData.h"


typedef struct urllist
{
	int type;//����
	map<int, wstring> url;//�ؼ������url����
}URLLIST, *LPURLLIST;

typedef struct BillUrlData
{
	string id;// �������
	wstring name;// ��������
	wstring dll;// ��̬������
	list<URLLIST> urldata;
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
	HMODULE m_urldll;
private:
	// �洢�����˵����������õ�����ַ������
	list<BILLURLDATA> m_billlist;
	// ��ȡ�����˵���ַ�����ļ�
	std::string GetFileContent(wstring strPath,bool bCHK);

	// �洢�͵����˵���������̺߳����ݣ���֤�ɵ����˵����̴߳��������߳�Ҳ���ҵ�Ҫ�����˵�����������������
	map<DWORD, BillData*>  m_billTid;

	CRITICAL_SECTION	m_cs; // �ٽ���,��ʱû��
public:	
	// ���ݻ���aid���˵����ͻ��������Ӧ��url�͹ؼ���������ݶ�Ӧ��ϵ
	map<int, wstring>*  GetUrlMap(string aid, int type);
	wstring GetBillUrl(string aid, int type, int step);

	bool InsertBillTid(DWORD pid, BillData* pData);
	BillData* GetBillTid(DWORD pid);
public:
	wstring GetDllName(string aid);
	bool DeleteBillTid(DWORD pid);
	void FreeDll();
	//CComVariant CallJScript(IHTMLDocument2 *doc, std::string strFunc,std::vector<std::string>& paramVec);
	//CComVariant CallJScript2(IHTMLDocument2 *doc, std::string strFunc, DISPPARAMS& dispparams);
	// ���˵�����д�����ݿ�Ľӿ�
	void WriteBillRecordToDB(BILLRECORD& TRecord);
public:
	// ����dll������ҳ���ݺͲ������ݵĽӿ�
	int GetBill(IWebBrowser2* pWebBrowser, BillData* pData, int step);

};