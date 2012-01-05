// eciticGetBill.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <string>
#include <map>
#include <list>
#include <vector>
#include "../../BankData/BankData.h"
#include "../../ThirdParty/RunLog/RunLog.h"
#include "ConvertBase.h"
using namespace std;

static wstring wstrEMPSID = L"";	// EMP_SID
static wstring wstrCardNo = L"";	// �������ÿ���
HWND g_notifyWnd = NULL;
int g_bState = 0;

#define CHECHSTATE if(g_bState != 0){state = g_bState;break;}
#define CHECKGETURLPAGEINFOSTATUS 	if(iRes != 0){state = BLII_NEED_RESTART;RecordInfo(L"ecitic", 1800, L"GetUrlPageInfoʧ��");CHECHSTATE;break;}

void WINAPI SetBillState(int State)
{
	g_bState = State;
}
void WINAPI SetNotifyWnd(HWND notifyWnd)
{
	g_notifyWnd = notifyWnd;
}

void ChangeNotifyWord(WCHAR* info)
{
	if(g_notifyWnd != 0)
		::PostMessage(g_notifyWnd, WM_BILL_CHANGE_NOFIFY, 0, (LPARAM)info);
}

void ShowNotifyWnd(bool bShow)
{
	if(g_notifyWnd != NULL)
	{
		if(!bShow)
			::SendMessage(g_notifyWnd, WM_BILL_HIDE_NOFIFY, 0, 0);
		else
			::SendMessage(g_notifyWnd, WM_BILL_SHOW_NOFIFY, 0, 0);
	}
}

bool RecordInfo(wstring program, DWORD common, wchar_t *format, ...)
{
	wchar_t strTemp[MAX_INFO_LENGTH];
	memset(strTemp, 0, sizeof(strTemp));
	wchar_t *pTemp = strTemp;
	//�ϳ���Ϣ
	va_list args; 
	va_start(args,format); 
	vswprintf(pTemp,MAX_INFO_LENGTH,format,args); 
	va_end(args); 

	wstring stemp(strTemp);


	wchar_t cinfo[20]= { 0 };
	swprintf(cinfo, 20, L"0x%08x", common);
	wstring wscTmp(cinfo);
	wscTmp  = program + L"-" + wscTmp + L"-" + stemp;

	CRunLog::GetInstance ()->GetLog ()->WriteSysLog (LOG_TYPE_INFO, L"%ws", wscTmp.c_str());
	return true;
}

void WINAPI FreeMemory(LPBILLRECORDS plRecords)
{
	list<LPBILLRECORD>::iterator ite = plRecords->BillRecordlist.begin();
	for(;ite != plRecords->BillRecordlist.end(); ite ++)
	{
		if((*ite) != NULL)
		{
			list<LPMONTHBILLRECORD>::iterator mite = (*ite)->bills.begin();
			for(;mite != (*ite)->bills.end(); mite ++)
			{
				list<LPTRANRECORD>::iterator lite = (*mite)->TranList.begin();
				for(; lite != (*mite)->TranList.end();lite ++)
				{
					if((*lite) != NULL)
						delete (*lite);
				}
				(*mite)->TranList.clear();
				delete (*mite);
			}
			(*ite)->bills.clear();

			delete (*ite);
		}
	}

	plRecords->BillRecordlist.clear();
	memset(plRecords->aid, 0, 256);
	plRecords->m_mapBack.clear();
	memset(plRecords->tag, 0, 256);
	plRecords->isFinish = false;
}

// ɾ��string�е�����ĳ�ַ�
void StringDeleteChar(string& str, char c)
{
	while (str.find(c) != string::npos)
	{
		string::size_type pos = str.find(c);
		str.erase(pos, 1);
	}
}

// ��ҳ���л�ȡ�·���Ϣ����ʽ��Ϊ201106
void GetMonthsInfo(string strPageInfo, LPBILLRECORDS plRecords, int &state)	
{
	////////////���ݸ�ʽbegin/////////////
	//////displayYMON("201110");</
	//////��ȡ201110�Ϳ���ֱ�Ӵ��ã����ø�ʽ�����ǳ����㡣
	////////////���ݸ�ʽend///////////

	string::size_type tPos = strPageInfo.find("displayYMON(\"");
	if (tPos == string::npos)
	{
		state = BILL_COM_ERROR;
		RecordInfo(L"ecitic", 1800, L"��ȡ�·���Ϣʧ��");
		return;
	}

	while (tPos != string::npos)
	{
		tPos += 13;
		string strMonthInfo = strPageInfo.substr(tPos, 6);
		SELECTINFONODE mNode;
		strncpy_s(mNode.szNodeInfo, 256, strMonthInfo.c_str(), strMonthInfo.length());
		mNode.dwVal = CHECKBOX_SHOW_UNCHECKED;
		plRecords->m_mapBack.push_back(mNode);
		tPos = strPageInfo.find("displayYMON(\"", tPos);
	}
	RecordInfo(L"ecitic", 1800, L"��ȡ�·���Ϣ�ɹ�");
}

/*
���ܣ�Ѱ��strPageInfo�д�ָ��startPos��ʼ�ĵ�n��ƥ����Ӵ�des���±�
������strPageInfo	-->Դ��
des				-->���ҵ��Ӵ�
desLength	-->�����Ӵ��ĳ���(C����ִ�������\0����)
startPos		-->��ʼ
n					-->��n��ƥ����Ӵ�
����ֵ�������ҳɹ����򷵻ض�Ӧ�±꣬��ʧ���򷵻�std::string::npos
*/
string::size_type StringAppointFind(string strPageInfo, char* des, int desLength, string::size_type startPos, int n)
{
	string::size_type resPos = startPos;

	int i = 0;

	while (i != n)
	{
		string::size_type tmpPos;
		if (resPos + desLength >= strPageInfo.size())
		{
			return std::string::npos;
		}
		tmpPos = strPageInfo.find(des, resPos + desLength);
		if (string::npos == tmpPos)
		{
			return std::string::npos;
		}
		resPos = tmpPos;
		i++;
	}

	return resPos;
}

// ��ȡRMB��USD��д�뵽plRecords
void GetAccountBalance(string strPageInfo, LPBILLRECORDS lpRecords)
{
	string::size_type startPos;
	string::size_type endPos;
	string strBalance;

	// ��ȡRMB���
	// �ڴ˼���ȡҳ��Դ���4��form_table_td2��ǩ�е�����
	// ��ʽ��<TD class="form_table_td2">&nbsp;0.00&nbsp;&nbsp;*&nbsp;������ʾ������</TD>
	LPBILLRECORD pRecord = new BILLRECORD;
	pRecord->balance = "F";
	pRecord->type = RMB;
	startPos = StringAppointFind(strPageInfo, "form_table_td2", 14, 0, 4);
	if (startPos == std::string::npos)
	{
		pRecord->balance = "F";
	}
	else
	{
		startPos += 22;	// form_table_td2">&nbsp;
		endPos = strPageInfo.find("&nbsp;", startPos);
		if (endPos == std::string::npos)
		{
			pRecord->balance = "F";
		}
		else
		{
			strBalance = strPageInfo.substr(startPos, endPos - startPos);
			// �������ÿ������Ϊ'-'�������Ǯ����'-'��ǷǮ�������ǵ��߼��෴������ȫ��ȡ������ȥ������е�','��
			if (strBalance.find("-") != string::npos)
			{
				StringDeleteChar(strBalance, '-');
				StringDeleteChar(strBalance, ',');
			}
			else
			{
				strBalance = '-' + strBalance;
				StringDeleteChar(strBalance, ',');
			}
			pRecord->balance = strBalance.c_str();
		}
	}
	lpRecords->BillRecordlist.push_back(pRecord);
	if (pRecord->balance == "F")
	{
		RecordInfo(L"ecitic", 1800, L"��ȡ��������ʧ��");
	}

	// ��ȡUSD���
	// �ڴ˼���ȡҳ��Դ���8��form_table_td2��ǩ�е�����
	// ��ʽ��<TD class="form_table_td2">&nbsp;0.00&nbsp;&nbsp;*&nbsp;������ʾ������</TD>
	LPBILLRECORD puRecord = new BILLRECORD;
	puRecord->balance = "F";
	puRecord->type = USD;
	startPos = StringAppointFind(strPageInfo, "form_table_td2", 14, 0, 8);
	if (startPos == std::string::npos)
	{
		puRecord->balance = "F";
	}
	else
	{
		startPos += 22; // form_table_td2">&nbsp;
		endPos = strPageInfo.find("&nbsp;", startPos);
		if (endPos == std::string::npos)
		{
			puRecord->balance = "F";
		}
		else
		{
			strBalance = strPageInfo.substr(startPos, endPos - startPos);
			// �������ÿ������Ϊ'-'�������Ǯ����'-'��ǷǮ�������ǵ��߼��෴������ȫ��ȡ������ȥ������е�','��
			if (strBalance.find("-") != string::npos)
			{
				StringDeleteChar(strBalance, '-');
				StringDeleteChar(strBalance, ',');
			}
			else
			{
				strBalance = '-' + strBalance;
				StringDeleteChar(strBalance, ',');
			}
			puRecord->balance = strBalance.c_str();
		}
	}
	lpRecords->BillRecordlist.push_back(puRecord);
	if (puRecord->balance == "F")
	{
		RecordInfo(L"ecitic", 1800, L"��ȡ��Ԫ���ʧ��");
	}
}

// �������е�����ȥ��
string FilterMonthStringNumber(const string& scr)
{
	string result;
	char *p = (char*)scr.c_str();
	char temp[2] = { 0 };
	for(unsigned char i = 0;i < scr.size();i ++)
	{		
		if((((*p) >= '0')&&((*p) <= '9'))) 
		{
			memcpy(&temp[0], p, 1);
			result += temp; 
		}
		p ++;
	}
	return result;
}

/*
���ܣ�����ҳ�����з�����ȡ����RMB��USD�˵���¼д�뵽plRecords
������strPageInfo		-->�˵���Ϣ��Դ
strTmpMonth	-->��Ϣ�����·�
billtype			-->���ױ��֣�RMB or USD
plRecords			-->�������ˣ��޴�����
*/
void GetRMBOrUSDBill(BillType billtype, string strPageInfo, string strTmpMonth,  LPBILLRECORDS plRecords)
{
	// �ҵ������ɵ�����һ�����Ԫ�˵�
	LPBILLRECORD pRecord = NULL;
	list<LPBILLRECORD>::iterator iter = plRecords->BillRecordlist.begin();
	for (; iter !=plRecords->BillRecordlist.end(); iter++)
	{
		if((*iter)->type == billtype)
		{
			pRecord = (*iter);
			break;
		}
	}

	LPMONTHBILLRECORD pMonthRecord = new MONTHBILLRECORD;
	pMonthRecord->month = strTmpMonth;	

	// �ж��Ƿ��ȡ������ȷ��ҳ����Ϣ
	// ����������м�¼���޼�¼��Դ���ж��С�ҳ��š�������¼��ʱ���ޡ�
	string::size_type nTmpPos = strPageInfo.find("ҳ���");
	if (string::npos == nTmpPos)
	{
		pMonthRecord->m_isSuccess = false;
		return;
	}
	//	<tr align="center" class="bg1"
	//	onmouseover="this.className ='result_over';"
	//	onmouseout="this.className ='bg1';">
	//	<td align = "center">2011��10��25��</td>
	//	<td align = "center">2011��10��27��</td>
	//	<td align = "left">PAYPAL *DIGITALRIVE WW  4029357733   SGP</td>
	//	<td align = "center">85326961299142117335514</td>
	//	<td align = "center">6897</td>
	//	<td align = "center">�����</td>
	//	<td align = "center">19,733.99</td>
	//	<td align = "center">19,733.99</td>
	//	</tr>

	nTmpPos = 0;
	int i = 0;
	LPTRANRECORD preRecord = NULL;
	while (string::npos != strPageInfo.find("<td align = \"", nTmpPos))
	{
		if (i%8 == 0)
		{
			preRecord = new TRANRECORD;
		}

		string::size_type beginPos = strPageInfo.find("<td align = \"", nTmpPos);
		string::size_type endPos = strPageInfo.find("</td>", beginPos);
		if (beginPos == string::npos || endPos == string::npos)
		{
			break;
		}
		nTmpPos = endPos;
		string strPer;
		if (i%8 == 2)
		{
			strPer = strPageInfo.substr(beginPos + 19, endPos - beginPos - 19);
		}
		else
		{
			strPer = strPageInfo.substr(beginPos + 21, endPos - beginPos - 21);
		}
		switch (i%8)
		{
		case 0:	// ��������
			strPer = FilterMonthStringNumber(strPer);
			sprintf_s(preRecord->TransDate, 256, "%s", strPer.c_str());
			break;
		case 1:	// ��������
			strPer = FilterMonthStringNumber(strPer);
			sprintf_s(preRecord->PostDate, 256, "%s", strPer.c_str());
			break;
		case 2:	// ��������
			sprintf_s(preRecord->Description, 256, "%s", strPer.c_str());
			break;
		case 3:	// �ο����
			break;
		case 4:	// ����ĩ��λ
			break;
		case 5:	// ���ױ���
			break;
		case 6:	// ���׽��
			StringDeleteChar(strPer, ',');
			sprintf_s(preRecord->Amount, 256, "%s", strPer.c_str());
			break;
		case 7:	//	������
			break;
		default:
			break;
		}

		i++;
		if (i % 8 ==0 && i != 0)
		{
			pMonthRecord->TranList.push_back(preRecord);
		}
	}

	pMonthRecord->m_isSuccess = true;

	if (i%8 != 0)	// ��ȡ�����в���������
	{
		pMonthRecord->TranList.clear();
		pMonthRecord->m_isSuccess = false;
	}

	pRecord->bills.push_back(pMonthRecord);
}

int TransferDataPost(wstring& page, wstring& cookie, wstring postdata, wstring referer, string& strPageInfo)
{
	USES_CONVERSION;
	string data = W2A(postdata.c_str());
	HINTERNET		m_hInetSession; // �Ự���
	HINTERNET		m_hInetConnection; // ���Ӿ��
	HINTERNET		m_hInetFile; //


	m_hInetSession = ::InternetOpen(L"Moneyhub3.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (m_hInetSession == NULL)
	{
		return 3000;
	}

	DWORD dwTimeOut = 5000;
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONTROL_SEND_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_SEND_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);


	m_hInetConnection = ::InternetConnect(m_hInetSession, L"e.bank.ecitic.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (m_hInetConnection == NULL)
	{
		InternetCloseHandle(m_hInetSession);

		return 3001;
	}

	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*"); 
	ppszAcceptTypes[1] = NULL;

	m_hInetFile = HttpOpenRequestW(m_hInetConnection, _T("POST"), page.c_str(), NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_SECURE /*| INTERNET_FLAG_DONT_CACHE*/ | INTERNET_FLAG_KEEP_CONNECTION, 0);
	if (m_hInetFile == NULL)
	{
		InternetCloseHandle(m_hInetConnection);
		InternetCloseHandle(m_hInetSession);
		return 3002;
	}	

	TCHAR szHeaders[1024];	
	_stprintf_s(szHeaders, _countof(szHeaders), _T("Cookie: %s;\r\nContent-Length: %d\r\n"), cookie.c_str(), postdata.length());


	BOOL ret = HttpAddRequestHeaders(m_hInetFile, szHeaders, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	wstring heads = L"Cache-Control: no-cache\r\nAccept: image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/x-silverlight, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, application/x-ms-application, application/x-ms-xbap, application/vnd.ms-xpsdocument, application/xaml+xml, */*\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: zh-cn\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; InfoPath.2; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)\r\nContent-Type: application/x-www-form-urlencoded\r\nReferer: " + referer + L"\r\nConnection: Keep-Alive\r\nHost: e.bank.ecitic.com\r\n";

	ret = HttpAddRequestHeadersW(m_hInetFile, heads.c_str(), -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE); 

	char sData[256] = {0};
	sprintf_s(sData , 256, "%s", data.c_str());
	BOOL bSend = ::HttpSendRequestW(m_hInetFile, NULL, 0, (void*)sData, strlen(sData));
	if (!bSend)
	{
		int err = GetLastError();
		InternetCloseHandle(m_hInetConnection);
		InternetCloseHandle(m_hInetFile);
		InternetCloseHandle(m_hInetSession);

		return err;
	}

	TCHAR szStatusCode[32];
	DWORD dwInfoSize = sizeof(szStatusCode);
	if (!HttpQueryInfo(m_hInetFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL))
	{
		InternetCloseHandle(m_hInetConnection);
		InternetCloseHandle(m_hInetFile);
		InternetCloseHandle(m_hInetSession);

		return 3004;
	}
	else
	{
		long nStatusCode = _ttol(szStatusCode);
		if (nStatusCode != HTTP_STATUS_PARTIAL_CONTENT && nStatusCode != HTTP_STATUS_OK)
		{
			InternetCloseHandle(m_hInetConnection);
			InternetCloseHandle(m_hInetFile);
			InternetCloseHandle(m_hInetSession);
			return 3005;
		}
	}

	const DWORD dwBytesToRead = 1024;
	char szReadBuf[dwBytesToRead];
	DWORD dwBytesRead;
	strPageInfo = "";

	do
	{
		BOOL bReadStatus = ::InternetReadFile(m_hInetFile, (LPVOID)szReadBuf, dwBytesToRead, &dwBytesRead);

		if (false == bReadStatus)
		{
			InternetCloseHandle(m_hInetConnection);
			InternetCloseHandle(m_hInetFile);
			InternetCloseHandle(m_hInetSession);
			return 3006;
		}

		string::size_type nTmpPos = strPageInfo.size();
		strPageInfo += szReadBuf;
		strPageInfo = strPageInfo.substr(0, nTmpPos +dwBytesRead);
	}while (std::string::npos == strPageInfo.find("</html>") && 0 != dwBytesRead);

	InternetCloseHandle(m_hInetConnection);
	InternetCloseHandle(m_hInetFile);
	InternetCloseHandle(m_hInetSession);

	return 0;
}

int WINAPI FetchBillFunc(IWebBrowser2* pFatherWebBrowser, IWebBrowser2* pChildWebBrowser, BillData* pData, int &step, LPBILLRECORDS plRecords)
{
	USES_CONVERSION;
	int state = 0;
	if(pData->aid == "a007")
	{
		RecordInfo(L"eciticDll", 1800, L"���뵼���˵�����%d", step);

		wstring wstrUrl;

		HRESULT hr = S_OK;
		IDispatch *docDisp			= NULL;
		IHTMLDocument2 *doc			= NULL;
		IHTMLDocument3 *doc3		= NULL;
		IHTMLElement *elem			= NULL;

		hr = pFatherWebBrowser->get_Document(&docDisp);
		if (SUCCEEDED(hr) && docDisp != NULL)
		{					
			hr = docDisp->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc));	
			if (SUCCEEDED(hr) && doc != NULL) 
			{
				if (docDisp) { docDisp->Release(); docDisp = NULL; }

				switch(step)
				{	
				case 1:// case 1������https://e.bank.ecitic.com/perbank5/signInCredit.do��ڲ�����ʲô��ֱ���������ÿ����档
					{
					}
					break;

				case 2:
					{
						BSTR pbUrl = NULL;
						pChildWebBrowser->get_LocationURL(&pbUrl);
						if(NULL != pbUrl)
						{
							wstring url = pbUrl;
							if(url.find(L"/perbank5/gotoCreditMain.do") != wstring::npos)	//����ҳ�������ɣ�����ȥ�����һ���������ˣ��ڴ��ǻ�ȡҳ�������·���Ϣ
							{
								// ��ȡ�˻����
								//Sleep(3000);
								BSTR pbCookie = NULL;
								doc->get_cookie(&pbCookie);
								wstring cookie = pbCookie;
								wstring postdata = L"cardNo=";
								postdata += wstrCardNo;
								wstring page = L"/perbank5/pb5640_maincardSettingreq.do?EMP_SID=";
								page += wstrEMPSID;
								wstring referer = L"https://e.bank.ecitic.com" + page;
								string strPageInfo;
								if(TransferDataPost(page, cookie, postdata, referer, strPageInfo) == 0)
								{
									GetAccountBalance(strPageInfo, plRecords);
									RecordInfo(L"ecitic", 1800, L"GetAccountBalance�ɹ�%d", step);
								}
								CHECHSTATE;
								

								// ��ȡ�·���Ϣ
								postdata = L"accountNo=" + wstrCardNo + L"&currencyType1=840";
								page = L"/perbank5/pb5130_returnOtherMonth.do?EMP_SID=" + wstrEMPSID;
								referer = L"https://e.bank.ecitic.com/perbank5/pb5130_historyBillQry.do?EMP_SID=" + wstrEMPSID;
								if(TransferDataPost(page, cookie, postdata, referer, strPageInfo) != 0)
									state = BILL_COM_ERROR;;
								GetMonthsInfo(strPageInfo, plRecords, state);
								CHECHSTATE;
								RecordInfo(L"ecitic", 1800, L"��ȡ�·���Ϣ�ɹ�%d", step);

								if (state != BILL_COM_ERROR)
									state = BILL_SELECT_MONTH;
							}
							else if (url.find(L"/perbank5/welcomeCredit.do") != wstring::npos)	// ��ҳ�棬���Ի�ȡEMP_SID���˺�ĩ��λ
							{
								CHECHSTATE;
								IDispatch *docDisp_c			= NULL;
								IHTMLDocument2 *doc_c			= NULL;

								hr = pChildWebBrowser->get_Document(&docDisp_c);
								if (SUCCEEDED(hr) && docDisp_c != NULL)
								{					
									hr = docDisp_c->QueryInterface(IID_IHTMLDocument2, reinterpret_cast<void**>(&doc_c));
									if (SUCCEEDED(hr) && doc_c != NULL) 
									{
										if (docDisp_c) { docDisp_c->Release(); docDisp_c = NULL; }
									}
								}

								hr = doc_c->get_body( &elem);

								wstring info;
								if(elem != NULL)   
								{      
									BSTR pbBody = NULL;
									elem->get_innerHTML(&pbBody); 
									if(pbBody != NULL)
										info = pbBody;
									elem->Release();   
									elem = NULL;
								}
								if (doc_c) { doc_c->Release(); doc_c = NULL; }

								wstring::size_type pos;
								wstring::size_type endpos;
								// ��ȡEMP_SID
								pos = info.find(L"EMP_SID");
								if (pos != wstring::npos)
								{
									wstrEMPSID = info.substr(pos + 8, 40);
								}
								if (wstrEMPSID == L"")
								{
									state = BILL_COM_ERROR;
									RecordInfo(L"ecitic", 1800, L"��ȡEMP_SIDʧ��%d", step);
								}
								CHECHSTATE;
								RecordInfo(L"ecitic", 1800, L"��ȡEMP_SID�ɹ�%d", step);

								// ��ȡ�����˺Ų����ô洢�ṹ�е��˺�ĩ��λ
								pos = info.find(L"accountNo=");
								if (pos != wstring::npos)
								{
									endpos = info.find(L"&amp", pos);
									if (endpos != wstring::npos)
									{
										wstrCardNo = info.substr(pos + 10, endpos - pos - 10);
									}
								}
								if (wstrCardNo == L"")
								{
									state = BILL_GET_ACCOUNT_ERROR;
									RecordInfo(L"ecitic", 1800, L"��ȡCARDNOʧ��%d", step);
								}
								else
								{
									wstring card = wstrCardNo.substr(wstrCardNo.length()-4, 4);
									sprintf_s(plRecords->tag, 256, W2A(card.c_str()));
								}
								CHECHSTATE;
								RecordInfo(L"ecitic", 1800, L"��ȡCARDNO�ɹ�%d", step);

								/////////////////�Ѱ��˻��뵱ǰ��½�˻��Ƿ�ƥ��begin//////////////////
								sprintf_s(plRecords->aid, 256,pData->aid.c_str());
								plRecords->type = pData->type;
								plRecords->accountid = pData->accountid;
								if(plRecords->tag != "" && pData->tag != "")
								{
									if(pData->tag != string(plRecords->tag))
									{
										ShowNotifyWnd(false);//�������ڵ�½��

										WCHAR sInfo[256] = { 0 };
										swprintf(sInfo, 256, L"��ǰ�˻��Ѱ����ſ���ĩ4λ%s��ʵ�ʵ������ÿ�����ԭ�˻���һ�£��Ƿ��������",A2W(pData->tag.c_str()));

										HWND hMainFrame = FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
										if(MessageBoxW(hMainFrame, sInfo, L"�ƽ��", MB_OKCANCEL) == IDCANCEL)
										{
											state = BLII_NEED_RESTART;
											break;
										}

										ShowNotifyWnd(true);//��ʾ���ڵ�½
									}								
								}
								/////////////////�Ѱ��˻��뵱ǰ��½�˻��Ƿ�ƥ��end//////////////////////
								CHECHSTATE;
							}
						}
					}
					break;

				case 3:	// ��ȡ������·���Ϣ
					{
						CHECHSTATE;

						step = 9; // �޸�step����ֹ��ε���
						// �����û�ѡ����·ݣ����˵����ݵ��������������Ԫ������

						//////////��ȡ�û�ѡ����·�begin////////////////
						list<string> selMonth;
						for (list<SELECTINFONODE>::iterator ite = plRecords->m_mapBack.begin();
							ite != plRecords->m_mapBack.end(); ite ++)
						{
							if (CHECKBOX_SHOW_CHECKED == ite->dwVal)
							{
								selMonth.push_back(ite->szNodeInfo);
							}
						}
						//////////��ȡ�û�ѡ����·�end//////////////////

						// ���������ļ�¼����·���Ϣ������ѡ���·ݣ�ֱ���˳�
						plRecords->m_mapBack.clear();
						if (0 == selMonth.size())
						{
							plRecords->isFinish = true;
							state = BILL_FINISH_STATE;
							break;
						}

						list<string>::iterator iter;
						BSTR pbCookie = NULL;
						doc->get_cookie(&pbCookie);
						wstring cookie = pbCookie;
						wstring postdata;
						wstring page;
						wstring referer;
						string strPageInfo;

						///////////////��ȡ������˵�begin///////////////////
						for (iter = selMonth.begin(); iter != selMonth.end(); iter++)
						{
							/*
							�޸���ʾ��Ϣ��
							�������ڻ�ȡ������˵���XXXX-XX-XX �� XXXX-XX-XX 
							*/
							wstring wstrTmp = A2W((*iter).c_str());
							wstring nInfo = L"���ڵ���RMB�˵���" + wstrTmp;
							ChangeNotifyWord((WCHAR*)nInfo.c_str());
							CHECHSTATE;

							postdata = L"accountNo=" + wstrCardNo + L"&accoMonth=" + A2W((*iter).c_str()) + L"&currencyType=156&startNo=1&crdRecordItem=500&opFlag=1&sFrnCyno=840&EMP_SID=" + wstrEMPSID;
							page = L"/perbank5/pb5130_invokeHisDetailBiz.do";
							referer = L"https://e.bank.ecitic.com/perbank5/pb5130_returnOtherMonth.do?EMP_SID=" + wstrEMPSID;
							strPageInfo.clear();
							if(TransferDataPost(page, cookie, postdata, referer, strPageInfo) == 0)								
								GetRMBOrUSDBill(RMB, strPageInfo, *iter, plRecords);
							CHECHSTATE;
						}
						///////////////��ȡ������˵�end/////////////////////
						if (state != 0)
						{
							state = BILL_CANCEL_GET_BILL;
							break;
						}
						RecordInfo(L"ecitic", 1800, L"��ȡ������˵���Ϣ���%d", step);

						///////////////��ȡ��Ԫ�˵�begin///////////////////
						for (iter = selMonth.begin(); iter != selMonth.end(); iter++)
						{
							/*
							�޸���ʾ��Ϣ��
							�������ڻ�ȡ��Ԫ�˵���XXXX-XX-XX �� XXXX-XX-XX 
							*/
							wstring wstrTmp = A2W((*iter).c_str());
							wstring nInfo = L"���ڵ���USD�˵���" + wstrTmp;
							ChangeNotifyWord((WCHAR*)nInfo.c_str());
							CHECHSTATE;

							postdata = L"accountNo=" + wstrCardNo + L"&accoMonth=" + A2W((*iter).c_str()) + L"&currencyType=840&startNo=1&crdRecordItem=500&opFlag=1&sFrnCyno=156&EMP_SID=" + wstrEMPSID;
							page = L"/perbank5/pb5130_invokeHisDetailBiz.do";
							referer = L"https://e.bank.ecitic.com/perbank5/pb5130_returnOtherMonth.do?EMP_SID=" + wstrEMPSID;
							strPageInfo.clear();
							if(TransferDataPost(page, cookie, postdata, referer, strPageInfo) == 0)								
								GetRMBOrUSDBill(USD, strPageInfo, *iter, plRecords);
							CHECHSTATE;
						}
						///////////////��ȡ��Ԫ�˵�end/////////////////////
						if (state != 0)
						{
							state = BILL_CANCEL_GET_BILL;
							break;
						}
						RecordInfo(L"ecitic", 1800, L"��ȡ������˵���Ϣ���%d", step);

						if (0 == g_bState)
						{
							plRecords->isFinish = true;
							state = BILL_FINISH_STATE;
						}
					}
					break;

				default:
					break;	
				}
			}
		}

		if (elem) { elem->Release(); elem = NULL; }
		if (doc3) { doc3->Release(); doc3 = NULL; }
		if (doc) { doc->Release(); doc = NULL; }
		if (docDisp) { docDisp->Release(); docDisp = NULL; }
	}
	return state;
}