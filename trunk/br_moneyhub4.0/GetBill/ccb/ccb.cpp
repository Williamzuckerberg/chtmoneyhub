// ccbGetBill.cpp : Defines the exported functions for the DLL application.
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

HWND g_notifyWnd = NULL;
int g_bState = 0;

#define CHECHSTATE if(g_bState != 0){state = g_bState;break;}
#define CHECKGETURLPAGEINFOSTATUS 	if(iRes != 0){state = BILL_COM_ERROR;RecordInfo(L"ccb", 1800, L"GetUrlPageInfoʧ��");break;}

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

int GetUrlPageInfo(WCHAR* url, string& strPageInfo)	// ��ȡָ��ҳ��HTML����
{
	HINTERNET hInetSession;	// �Ự���
	HINTERNET hInetFile;

	wstring revinfo;

	hInetSession = ::InternetOpen(L"Moneyhub3.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (NULL == hInetSession)
	{
		return 3000;
	}

	hInetFile = ::InternetOpenUrl(hInetSession, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (NULL == hInetFile)
	{
		::InternetCloseHandle(hInetSession);
		return 3002;
	}

	const DWORD dwBytesToRead = 1024;
	char szReadBuf[dwBytesToRead];
	DWORD dwBytesRead;
	strPageInfo = "";

	do
	{
		BOOL bReadStatus = ::InternetReadFile(hInetFile, (LPVOID)szReadBuf, dwBytesToRead, &dwBytesRead);

		if (false == bReadStatus)
		{
			//DWORD errorCode = ::GetLastError();
			::InternetCloseHandle(hInetFile);
			::InternetCloseHandle(hInetSession);
			return 3002;
		}

		string::size_type nTmpPos = strPageInfo.size();
		strPageInfo += szReadBuf;
		strPageInfo = strPageInfo.substr(0, nTmpPos +dwBytesRead);
	}while (std::string::npos == strPageInfo.find("</html>") && 0 != dwBytesRead);

	::InternetCloseHandle(hInetFile);
	::InternetCloseHandle(hInetSession);

	if (std::string::npos == strPageInfo.find("</html>"))
	{
		RecordInfo(L"ccb", 1800, L"GetUrlPageInfoʧ��");
	}

	return 0;
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

static std::map<string, int> mapMonthInfo;	// �������·���Ϣ���ɼ�ֵ�ԣ���FetchBillFunc��ץȡ�˵�ʱ���õ�

// ��ҳ���л�ȡ�·���Ϣ����ʽ��Ϊ201106
void GetMonthsInfo(string strPageInfo, LPBILLRECORDS plRecords)	
{
	////////////���ݸ�ʽbegin/////////////
	//<select name="sel_stcycles" >
	//	<option value="1">2011-11-08 �� 2011-12-07</option>
	//	<option value="2">2011-10-08 �� 2011-11-07</option>
	//	<option value="3">2011-09-08 �� 2011-10-07</option>
	//	<option value="4">2011-08-08 �� 2011-09-07</option>
	//	<option value="5">2011-07-08 �� 2011-08-07</option>
	//	<option value="6">2011-06-08 �� 2011-07-07</option>
	////////////���ݸ�ʽend///////////

	string::size_type tPos = strPageInfo.find("sel_stcycles");
	if (tPos == string::npos)
	{
		RecordInfo(L"ccb", 1800, L"��ȡ�·���Ϣʧ��");
		return;
	}

	tPos += 100;

	for (int i = 0; i < 6; ++i)
	{
		string strMonthInfo = strPageInfo.substr(tPos - 24, 24);

		if (tPos != strPageInfo.find("</option>", tPos + 10))
		{
			tPos = strPageInfo.find("</option>", tPos + 10);
			if (tPos == string::npos && i != 5)
				break;
		}
		////////////��ʽ���·�begin
		StringDeleteChar(strMonthInfo, '-');
		strMonthInfo = strMonthInfo.substr(0, 6);
		////////////��ʽ���·�end

		mapMonthInfo.insert(make_pair(strMonthInfo, i + 1));

		SELECTINFONODE mNode;
		strncpy_s(mNode.szNodeInfo, 256, strMonthInfo.c_str(), strMonthInfo.length());
		mNode.dwVal = CHECKBOX_SHOW_UNCHECKED;
		plRecords->m_mapBack.push_back(mNode);
	}
	if (mapMonthInfo.size() == 6)
	{
		RecordInfo(L"ccb", 1800, L"��ȡ�·���Ϣ�ɹ�");
	}
	else
	{
		RecordInfo(L"ccb", 1800, L"��ȡ�·���Ϣʧ��");
	}
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

// ��ȡ�˺�ĩ��λ
void GetAccount(string strPageInfo, LPBILLRECORDS plRecords, int& state)
{
	if (strPageInfo.find("****") == string::npos)
	{
		RecordInfo(L"ccb", 1800, L"��ȡҳ���˻�ʧ��");
		state = BILL_GET_ACCOUNT_ERROR;
	}
	else
	{
		string::size_type pos = strPageInfo.find("****");
		string card = strPageInfo.substr(pos+4, 4);
		strcpy_s(plRecords->tag, 256, card.c_str());
		RecordInfo(L"ccb", 1800, L"��ȡҳ���˻��ɹ�");
	}
}

// ��ȡRMB��USD��д�뵽plRecords
void GetAccountBalance(string strPageInfo, LPBILLRECORDS lpRecords)
{
	string::size_type startPos;
	string::size_type endPos;
	string strBalance;

	// ��ȡRMB���
	// �ڴ˼���ȡmain_card_ope.gspҳ��Դ���5��dline��ǩ�е�����
	// ��ʽ��<td id="dline">&nbsp;-4.14</td>
	LPBILLRECORD pRecord = new BILLRECORD;
	pRecord->balance = "F";
	pRecord->type = RMB;
	startPos = StringAppointFind(strPageInfo, "dline", 5, 0, 5);
	if (startPos == std::string::npos)
	{
		pRecord->balance = "F";
	}
	else
	{
		startPos += 13;	// dline">&nbsp;
		endPos = strPageInfo.find("</td>", startPos);
		if (endPos == std::string::npos)
		{
			pRecord->balance = "F";
		}
		else if (endPos == startPos)
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
				StringDeleteChar(strBalance, ',');
				if(strBalance != "0.00")
					strBalance = '-' + strBalance;
			}
			pRecord->balance = strBalance.c_str();
		}
	}
	lpRecords->BillRecordlist.push_back(pRecord);
	if (pRecord->balance == "F")
	{
		RecordInfo(L"ccb", 1800, L"��ȡ��������ʧ��");
	}

	// ��ȡUSD���
	// �ڴ˼���ȡmain_card_ope.gspҳ��Դ���10��dline��ǩ�е�����
	// ��ʽ��<td id="dline">&nbsp;0.00</td>
	LPBILLRECORD puRecord = new BILLRECORD;
	puRecord->balance = "F";
	puRecord->type = USD;
	startPos = StringAppointFind(strPageInfo, "dline", 5, 0, 10);
	if (startPos == std::string::npos)
	{
		puRecord->balance = "F";
	}
	else
	{
		startPos += 13; // dline">&nbsp;
		endPos = strPageInfo.find("</td>", startPos);
		if (endPos == std::string::npos)
		{
			puRecord->balance = "F";
		}
		else if (endPos == startPos)
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
				StringDeleteChar(strBalance, ',');
				if(strBalance != "0.00")
					strBalance = '-' + strBalance;
			}
			puRecord->balance = strBalance.c_str();
		}
	}
	lpRecords->BillRecordlist.push_back(puRecord);
	if (puRecord->balance == "F")
	{
		RecordInfo(L"ccb", 1800, L"��ȡ��Ԫ���ʧ��");
	}
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
	pMonthRecord->m_isSuccess = false;

	// �ж��Ƿ��ȡ������ȷ��ҳ����Ϣ
	string::size_type nTmpPos = strPageInfo.find("���ÿ��ѳ��˵���ϸ");
	if (string::npos == nTmpPos)
	{
		pMonthRecord->m_isSuccess = false;
		return;
	}

	// ���ݸ�ʽDEMO��5��dlineΪһ��
	//<tr onMouseOver="this.className='table_select_bg'" onMouseOut="this.className=''"  >
	//	<td id="dline">2011-07-24</td>
	//	<td id="dline">2011-07-24</td>
	//	<td id="dline">6.4633���㻹��</td>
	//	<td id="dline">��Ԫ</td>
	//	<td id="dline">246.22</td>
	//	</tr>
	nTmpPos = 0;
	int i = 0;
	LPTRANRECORD preRecord = NULL;
	while (string::npos != strPageInfo.find("dline", nTmpPos))
	{
		if (i%5 == 0)
		{
			preRecord = new TRANRECORD;
		}

		string::size_type beginPos = strPageInfo.find("dline", nTmpPos);
		string::size_type endPos = string::npos;
		if(beginPos != string::npos)
			endPos = strPageInfo.find("</td>", beginPos);
		if (beginPos == string::npos || endPos == string::npos)
		{
			break;
		}
		nTmpPos = endPos;
		string strPer = strPageInfo.substr(beginPos + 7, endPos - beginPos - 7);
		switch (i%5)
		{
		case 0:	// ������
			StringDeleteChar(strPer, '-');
			sprintf_s(preRecord->TransDate, 256, "%s", strPer.c_str());
			break;
		case 1:	// ������
			StringDeleteChar(strPer, '-');
			sprintf_s(preRecord->PostDate, 256, "%s", strPer.c_str());
			break;
		case 2:	// ������ϸ
			sprintf_s(preRecord->Description, 256, "%s", strPer.c_str());
			break;
		case 3:	// �������
			break;
		case 4:	// ���׽��
			{
				// ȥ����
				StringDeleteChar(strPer, ',');

				if (string(preRecord->Description).find("����") != string::npos)	// �����ÿ������Ϊ֧����֧��Ϊ+������Ϊ��
				{
					strPer = "-" + strPer;
				}
				sprintf_s(preRecord->Amount, 256, "%s", strPer.c_str());
			}
			break;
		default:
			break;
		}

		i++;
		if (i % 5 ==0 && i != 0)
		{
			pMonthRecord->TranList.push_back(preRecord);
		}
	}

	pMonthRecord->m_isSuccess = true;

	if (i%5 != 0)	// ��ȡ�����в���������
	{
		pMonthRecord->TranList.clear();
		pMonthRecord->m_isSuccess = false;
	}

	pRecord->bills.push_back(pMonthRecord);
}

// ��ֵ��MAP�л�ȡ��
string GetKeyFromMapByValue(int mapValue)
{
	std::map<string ,int>::iterator iter = mapMonthInfo.begin();
	while (iter != mapMonthInfo.end())
	{
		if (iter->second == mapValue)
		{
			return iter->first;
		}
		iter++;
	}
	return string("");
}

int WINAPI FetchBillFunc(IWebBrowser2* pFatherWebBrowser, IWebBrowser2* pChildWebBrowser, BillData* pData, int &step, LPBILLRECORDS plRecords)
{
	USES_CONVERSION;
	int state = 0;
	int iRes = 10000;	// ������stepΪ2��3ʱ��GetUrlPageInfo�ķ���ֵ�ж�
	if(pData->aid == "a004")
	{
		RecordInfo(L"ccbDll", 1800, L"���뵼���˵�����%d", step);

		switch(step)
		{	
		case 1:// case 1������http://accounts.ccb.com/accounts/login_card_ope.gsp��ڲ�����ʲô��ֱ���������ÿ����档
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
					if(url.find(L"/accounts/main_card_ope.gsp") != wstring::npos)
					{
						string strPageInfo;
						iRes = GetUrlPageInfo(L"http://accounts.ccb.com/accounts/main_card_ope.gsp", strPageInfo);
						CHECKGETURLPAGEINFOSTATUS;

						// ��ȡ�˺�ĩ��λ
						GetAccount(strPageInfo, plRecords, state);

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
								swprintf(sInfo, 256, L"��ǰ�˻��Ѱ󶨽��п���ĩ4λ%s��ʵ�ʵ������п�����ԭ�˻���һ�£��Ƿ��������",A2W(pData->tag.c_str()));

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

						// ��ȡ�˻����
						GetAccountBalance(strPageInfo, plRecords);

						// ��ȡ�·���Ϣ
						iRes = GetUrlPageInfo(L"http://accounts.ccb.com/accounts/card_already_ope.gsp", strPageInfo);
						CHECKGETURLPAGEINFOSTATUS;

						GetMonthsInfo(strPageInfo, plRecords);
						if(state != BILL_COM_ERROR)
							state = BILL_SELECT_MONTH;
						step = 3;
					}
				}
			}
			break;

		case 4:
			{
				CHECHSTATE;
				step = 9; // �޸�step����ֹ��ε���
				// �����û�ѡ����·ݣ����˵����ݵ��������������Ԫ������

				//////////��ȡ�û�ѡ����·�begin////////////////
				list<int> selMonth;
				for (list<SELECTINFONODE>::iterator ite = plRecords->m_mapBack.begin();
					ite != plRecords->m_mapBack.end(); ite ++)
				{
					if (CHECKBOX_SHOW_CHECKED == ite->dwVal)
					{
						int aMonth = mapMonthInfo[ite->szNodeInfo];
						selMonth.push_back(aMonth);
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

				//char* cCurType;	// ���֣�01��ʾ����ң�14��ʾ��Ԫ
				list<int>::iterator iter;
				wstring wstrPageUrl;
				string strPageInfo;

				//////////��ȡ������˵�begin////////////////////
				//cCurType = "01";
				wstrPageUrl.clear();

				for (iter = selMonth.begin(); iter != selMonth.end(); iter ++)
				{
					wstrPageUrl = L"http://accounts.ccb.com/accounts/card_already_main_ope.gsp?sel_currcodes=01&sel_stcycles=";

					/*
					�޸���ʾ��Ϣ��
					�������ڻ�ȡ������˵���XXXX-XX-XX �� XXXX-XX-XX 
					*/
					std::string strTmpMonth = GetKeyFromMapByValue(*iter);
					wstring wstrTmp = A2W(strTmpMonth.c_str());
					wstring nInfo = L"���ڵ���RMB�˵���" + wstrTmp;
					ChangeNotifyWord((WCHAR*)nInfo.c_str());

					CHECHSTATE;

					char cTmp = *iter + 48;
					wstrPageUrl += cTmp;
					strPageInfo.clear();
					iRes = GetUrlPageInfo((WCHAR*)wstrPageUrl.c_str(), strPageInfo);
					if(iRes == 0)
						GetRMBOrUSDBill(RMB, strPageInfo, strTmpMonth, plRecords);
				}

				if (state != 0)
				{
					state = BILL_CANCEL_GET_BILL;
					break;
				}

				//////////��ȡ������˵�end//////////////////////
				RecordInfo(L"ccb", 1800, L"��ȡ������˵���Ϣ���");

				//////////��ȡ��Ԫ�˵�begin//////////////////////
				//cCurType = "14";
				wstrPageUrl.clear();

				for (iter = selMonth.begin(); iter != selMonth.end(); iter ++)
				{
					wstrPageUrl = L"http://accounts.ccb.com/accounts/card_already_main_ope.gsp?sel_currcodes=14&sel_stcycles=";

					/*
					�޸���ʾ��Ϣ��
					���磺���ڻ�ȡ��Ԫ�˵���XXXX-XX-XX �� XXXX-XX-XX 
					*/
					std::string strTmpMonth = GetKeyFromMapByValue(*iter);
					wstring wstrTmp = A2W(strTmpMonth.c_str());
					wstring nInfo = L"���ڵ���USD�˵���" + wstrTmp;
					ChangeNotifyWord((WCHAR*)nInfo.c_str());

					CHECHSTATE;

					char cTmp = *iter + 48;
					wstrPageUrl += cTmp;
					strPageInfo.clear();
					iRes = GetUrlPageInfo((WCHAR*)wstrPageUrl.c_str(), strPageInfo);
					if(iRes == 0)
						GetRMBOrUSDBill(USD, strPageInfo, strTmpMonth, plRecords);
				}

				if (state != 0)
				{
					state = BILL_CANCEL_GET_BILL;
					break;
				}
				//////////��ȡ��Ԫ�˵�end////////////////////////
				RecordInfo(L"ccb", 1800, L"��ȡ��Ԫ�˵���Ϣ���");

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
	return state;
}