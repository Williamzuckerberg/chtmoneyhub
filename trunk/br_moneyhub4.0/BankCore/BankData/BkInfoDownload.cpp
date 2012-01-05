#include "stdafx.h"
#include "BkInfoDownload.h"
#include "../Utils/CryptHash/base64.h"
#include "../ThirdParty/tinyxml/tinyxml.h"
#include "../Utils/Config/HostConfig.h"
#include "../Utils/HardwareID/genhwid.h"
#include "../BankUI/Util/SecurityCheck.h"
#include "../Utils/FavBankOperator/FavBankOperator.h"
#include "../USBControl/USBCheckor.h"
#include "..\Security\Authentication\BankMdrVerifier\export.h"
#include "../Utils/ListManager/ListManager.h"
#include "../ExternalDispatchImpl.h"
#include "../../USBControl/USBHardWareDetector.h"
#include "../../BankUI/UIControl/CoolMessageBox.h"

#define _DownLoadPath  _T("BankDownLoad\\")
#define _SetUpSubPath _T("BankInfo\\banks\\")


CNotifyFavProgress* CNotifyFavProgress::m_Instance = NULL;
CNotifyFavProgress::CNotifyFavProgress():m_hwndNotify(NULL)
{
}

void CNotifyFavProgress::SetFavProgress(string appId, int progress, nsDownStates dstate, bool allowState)
{
	if(m_hwndNotify)
	{
		LPDOWNLOADSTATUS pstatus = new DOWNLOADSTATUS;
		pstatus->appId = appId;

		char temp[100] = {0};
		sprintf_s(temp, 100 , "%d", progress);
		string stp(temp);

		pstatus->progress = stp;

		switch(dstate)
		{
		case nsDownloading:pstatus->status = "��������";
			break;
		case nsInstalling:pstatus->status = "���ڰ�װ";
			break;
		default:pstatus->status = "";
			break;
		}

		switch(allowState)
		{
		case true:pstatus->logo = "1";
			break;
		default:pstatus->logo = "0";
			break;
		}
		
		::PostMessage (m_hwndNotify, WM_AX_FRAME_CHANGE_PROGRESS, (WPARAM)pstatus, 0);

	}
}
void CNotifyFavProgress::SetFavProgress(string appId, int progress)
{
	//if(progress == )
}
void CNotifyFavProgress::SetProgressNotifyHwnd(HWND hNotify)
{
	m_hwndNotify = hNotify;
}

void CNotifyFavProgress::CancelFav(string appId)//ȡ���ղ�
{
	if(m_hwndNotify)
	{
		LPARAM param;
		memcpy(&param , appId.c_str() ,sizeof(param));
		::PostMessage (m_hwndNotify, WM_AX_FRAME_DELETEFAV, 0, param);
	}
}

void CNotifyFavProgress::AddFav(string appId)// �����ղ�
{

	if(m_hwndNotify)
	{
		LPARAM param;
		memcpy(&param , appId.c_str() ,sizeof(param));
	
		::PostMessage (m_hwndNotify, WM_AX_FRAME_ADDFAV, 0, param);
	}
}

CNotifyFavProgress* CNotifyFavProgress::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CNotifyFavProgress();
	return m_Instance;
}



CBkInfoDownloadManager* CBkInfoDownloadManager::m_staticInstance = NULL;
IBankDownInterface* CBkInfoDownloadManager::m_staticpICheckFile = NULL;

CBkInfoDownloadManager::CBkInfoDownloadManager() :m_bAddBkCtrl(false)
{
	// ��ʼ���ٽ���
	InitializeCriticalSection(&m_cs);

	ReadAcquiesceSetupPath ();
	// ��ʱ�������ԣ�Ҫ����GetHWIDs
	m_strHWID = CA2W(GenHWID2().c_str ());
	DWORD dwThreadID = 0;
	CloseHandle (CreateThread (NULL, 0, CheckThreadProc, (LPVOID)this, 0, &dwThreadID));
}

CBkInfoDownloadManager::~CBkInfoDownloadManager()
{
	DeleteCriticalSection(&m_cs);
}

// ��ȡ�ļ�����
int CBkInfoDownloadManager::ReadDownLoadPercent(LPSTR lpBankID)
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return ERR_UNKNOW;

	CDownloadManagerThread* pTempThread = FindBankCtrlDLManager(lpBankID);
	if (NULL == pTempThread)
		return 200; // ��ʾ��װ���

	return pTempThread->GetAverageDownLoadProcess (); // ��ȡ����
}

void CBkInfoDownloadManager::MyBankCtrlDownload(LPSTR lpBankID)
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return;

	//OutputDebugString(L"�ղ����С������ں�");
	//OutputDebugString(CA2W (lpBankID));

	// ����Ѿ���װ�ˣ�����������
	if (IsSetupAlready (lpBankID))
	{
		NotifyCoreBankCtrlDLFinish (lpBankID);

		// �����ݿ��е����ݱ�ǳ��Ѿ���װ
		CBankData::GetInstance ()->SaveFav (lpBankID, 200);
		// �����ݿ��е����ݱ�ǳ��Ѿ���װ
		CNotifyFavProgress::GetInstance()->SetFavProgress(lpBankID, 200, nsNULL, true);
		return;
	}


	// �õ�ID�����ؿؼ�����ָ��
	CDownloadManagerThread* pTempThread = FindBankCtrlDLManager(lpBankID);

	if (NULL == pTempThread)
	{
		pTempThread = new CDownloadManagerThread (); // ����һ���������

		::EnterCriticalSection(&m_cs);
		m_BankManager.insert (std::make_pair (lpBankID, pTempThread)); // �������ɵļ��뵽map
		::LeaveCriticalSection (&m_cs);

	}
	else // ��������е��Ƿ���������
	{
		if (pTempThread->AddBankCtrlAlready ())
		{
			return;
		}
	}


	// ���ý���
	CNotifyFavProgress::GetInstance()->SetFavProgress(lpBankID, 0, nsDownloading, false);
	CBankData::GetInstance ()->SaveFav (lpBankID, 0);

	// �õ����ص���ʱ·��
	if (m_wstrDLTempPath.empty ())
		m_wstrDLTempPath = GetTempCachePath ();
	std::wstring strXmlPath = m_wstrDLTempPath;
	std::wstring strCtrlPath = m_wstrDLTempPath;	
	std::string strBankName = CFavBankOperator::GetBankIDOrBankName (lpBankID);
	strXmlPath += CA2W (strBankName.c_str ());
	strXmlPath += L".xml";
	strCtrlPath += CA2W (strBankName.c_str ());
	strCtrlPath += L".cab";

	// ��ȡ�ϵ�У����
	std::string strOldCk;
	std::list<std::wstring> TpList;
	if (!CheckBankCtrlXml(strXmlPath.c_str (), TpList, strOldCk)) 
	{
		strOldCk.clear ();
	}

	// �����ͷ�����ͨѶ��xml�ļ�
	std::string strTemp;
	ConstructBkCtrlCommunicateXml (lpBankID, strTemp);

	std::wstring downPath = CHostContainer::GetInstance()->GetHostName(kPDownloadInstall);
	DOWN_LOAD_PARAM_NODE myNode;
	myNode.bCreateThread = false;
	myNode.strHWID = m_strHWID;
	myNode.strSendData = strTemp;
	//myNode.dwPostDataLength = strTemp.size ();
	myNode.strSaveFile = strXmlPath;
	myNode.strUrl = downPath;



	std::string strBkIDTp = lpBankID;
	while (true && CExternalDispatchImpl::IsInUserFavBankList(strBkIDTp))
	{
		// �ӷ����������ؼ�������������ӣ�����������ʽ���أ�û�д����µ��߳�����,��ΪҪУ���룩
		if (!pTempThread->CreateDownLoadTask (&myNode))// (m_strHWID.c_str (), downPath.c_str (), strXmlPath.c_str (), (LPVOID)strTemp.c_str (), strTemp.size (), false)
		{
			//OutputDebugString(L"�ӷ���������XML����ʧ�ܣ��������ں�");
			CRecordProgram::GetInstance()->FeedbackError(MY_ERROR_PRO_CORE, ERR_DOWNLOAD_XML_CTRL, L"MyBankCtrlDownload�ӷ���������XML����ʧ�ܣ�");
			Sleep (5000);
			continue;
		}

		// ������֤�룬���õ���������
		std::string strNewCk;
		std::list<std::wstring> UrlList;
		if (!CheckBankCtrlXml(strXmlPath.c_str (), UrlList, strNewCk))
		{
			//OutputDebugString(L"У�����ص�XML����ʧ�ܣ��������ں�");
			CRecordProgram::GetInstance( )->FeedbackError(MY_ERROR_PRO_CORE, ERR_READ_XML_CTRL, L"MyBankCtrlDownloadУ�����ص�XML����ʧ�ܣ�");
			Sleep (5000);
			continue;
		}

		// ���û��У����Ͳ��Ƚ���
		if (!strOldCk.empty ()) 
		{
			// ����У���벻һ��˵��������������
			if (strOldCk != strNewCk) 
			{
				std::wstring strTp;
				CDownloadThread::TranslanteToBreakDownloadName (strXmlPath, strTp);
				DeleteFile (strTp.c_str ()); // �������ǰ�ºõ���ʱ�ļ�
			}
		}

		std::list<std::wstring>::iterator it;
		if (UrlList.empty ())
		{
			//OutputDebugString(L"��ȡ��������������Ϊ�գ��������ں�");
			CRecordProgram::GetInstance()->FeedbackError(MY_ERROR_PRO_CORE, ERR_DOWNLOAD_URL_NOFOUND_CTRL, CRecordProgram::GetInstance()->GetRecordInfo(L"MyBankCtrlDownload��ȡ%s������������Ϊ�գ�", strXmlPath.c_str()));
			
		}
		for (it = UrlList.begin (); it != UrlList.end (); it ++) // ������������
		{
			DOWN_LOAD_PARAM_NODE myNode;
			myNode.bCreateThread = true;
			myNode.strHWID = m_strHWID;
			if (!m_bAddBkCtrl)
				myNode.emKind = emBkActiveX;// ���ص������пؼ�
			else
			{
				myNode.emKind = emUSBBkActiveX; // ����USBKEY�������������ص����пؼ�
				m_bAddBkCtrl = false;
			}
			
			myNode.strSaveFile = strCtrlPath;
			myNode.strSetupPath = m_wstrSetupPath;
			myNode.bSetupDlFile = true;
			myNode.strUrl = (*it);

			pTempThread->CreateDownLoadTask (&myNode);
		}

		// �Ѿ��ɹ������������
		if (UrlList.size() > 0)
			return;

		// 5���������������
		Sleep(5000);
	}
}

void CBkInfoDownloadManager::MyBankUsbKeyDownload(LPUSBRECORD pUsbNode) // ����USBKEY// LPSTR lpBankID, LPVOID lpVoid, int nSize
{
	std::string strBankID = pUsbNode->fav;
	std::string strXml = pUsbNode->xml;
	std::string strVer = pUsbNode->ver;

	//������пؼ��Ƿ��Ѿ���װ
	CBankData* pBankData = CBankData::GetInstance();

	// 2����������һ���ͽ��а�װ
	bool bAddBkCtrl = false;
	CBkInfoDownloadManager* pManager = CBkInfoDownloadManager::GetInstance ();
	bool bSetup = pManager->IsSetupAlready (pUsbNode->fav);
	bool bExist = pBankData->IsFavExist(strBankID);
	if ((!bSetup) || (!bExist))
	{
		m_bAddBkCtrl = true; // ������ص����пؼ���USBKEY������
		CNotifyFavProgress::GetInstance()->AddFav(pUsbNode->fav);//����js��ʼ��װ
		
		bAddBkCtrl = true;
	}

	// �õ�USBKEY��������֣�VID+PID+MID
	std::wstring strSaveName;
	wchar_t wchTemp[20] = {0};
	_itow_s (pUsbNode->vid, wchTemp, 19, 10);
	strSaveName += wchTemp;
	memset (wchTemp, 0, sizeof (wchTemp));
	_itow_s (pUsbNode->pid, wchTemp, 19, 10);
	strSaveName += wchTemp;
	memset (wchTemp, 0, sizeof (wchTemp));
	_itow_s (pUsbNode->mid, wchTemp, 19, 10);
	strSaveName += wchTemp;

	// �õ�ID�����ؿؼ�����ָ��
	CDownloadManagerThread* pTempThread = FindBankCtrlDLManager((LPSTR)strBankID.c_str ());
	if (NULL == pTempThread)
	{
		pTempThread = new CDownloadManagerThread (); // ����һ���������

		::EnterCriticalSection(&m_cs);
		m_BankManager.insert (std::make_pair (strBankID.c_str (), pTempThread)); // �������ɵļ��뵽map
		::LeaveCriticalSection (&m_cs);
	}
	std::wstring strXmlPath, strCtrlPath;
	if (m_wstrDLTempPath.empty ())
		m_wstrDLTempPath = GetTempCachePath (); // �õ����ص���ʱ·��

	strXmlPath = m_wstrDLTempPath;
	strCtrlPath = m_wstrDLTempPath;

	strXmlPath += strSaveName;
	strXmlPath += L".xml";
	strCtrlPath += strSaveName;
	strCtrlPath += L".exe";

	//��base64����
	DWORD dwBufLen = strlen(strXml.c_str()) + 32;
	LPSTR lpszXmlInfo = new char[dwBufLen];

	strcpy_s(lpszXmlInfo,dwBufLen,strXml.c_str());

	int dwSize = strlen(lpszXmlInfo) * 2 + 1;
	unsigned char* pszOut = new unsigned char[dwSize];
	base64_encode((LPBYTE)lpszXmlInfo, strlen(lpszXmlInfo), pszOut, &dwSize);
	pszOut[dwSize] = 0;


	strXml = "xml=";
	strXml += UrlEncode((char *)pszOut);

	// �ӷ����������ؼ�������������ӣ�����������ʽ���أ�û�д����µ��߳�����,��ΪҪУ���룩
	std::wstring downPath = CHostContainer::GetInstance()->GetHostName(kPUkey);
	DOWN_LOAD_PARAM_NODE myNode;
	myNode.bCreateThread = false;
	myNode.strHWID = m_strHWID;
	myNode.strSendData = strXml;
	//myNode.dwPostDataLength = strXml.size ();
	myNode.strSaveFile = strXmlPath;
	myNode.strUrl = downPath;

	
	if (!pTempThread->CreateDownLoadTask (&myNode))
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_ERROR_PRO_CORE, ERR_DOWNLOAD_XML_USBKEY, L"MyBankUsbKeyDownload�ӷ�����������XML�ļ�ʧ�ܣ�");
	}

	// ������֤�룬���õ���������
	std::string strNewCk;
	std::string strVersion;
	std::list<std::wstring> UrlList;
	if (!CheckUsbKeyXml(strXmlPath.c_str (), UrlList, strNewCk, strVersion))
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_ERROR_PRO_CORE, ERR_READ_XML_USBKEY, CRecordProgram::GetInstance()->GetRecordInfo(L"MyBankUsbKeyDownload����%s����ʧ�ܣ�",strXmlPath.c_str ()));
	}

	bool bUpData = false;
	// ���а汾�Ƚ�
	int nVerComp = MyTwoVersionCompare (strVersion, strVer);
	// ��ʾҪ�����汾Ҫ���µ���ʾ
	if (nVerComp > 0)
		bUpData = true;
	if(strVer == DEFAULT_INSTALL_VERSION)
	{
		bUpData = false;
	}


	// �����usb�ؼ��Ƿ��Ѿ���װ��û�а�װ�����ذ�װ
	bool bCheck = m_staticpICheckFile->CheckServerXmlFile (bUpData, *pUsbNode);
	if(bCheck == false)
		return;
	int nStatus = pUsbNode->status; // ��������ж��Ƿ�װ
	if (200 == nStatus)
	{
		// ������пؼ��Ѿ��ղ�
		if (bAddBkCtrl == false || bSetup == true || bUpData == false)
		{
			USBKeyInfo* pusb = CUSBHardWareDetector::GetInstance()->CheckUSBHardWare(pUsbNode->vid, pUsbNode->pid, pUsbNode->mid);//���ڴ��ļ��л�������Ϣ

			if(!pusb)
				return;

			wstring msg;
			msg = pusb->hardware.goodsname + L"�����װ����������ʹ��";
			mhShowMessage( GetActiveWindow(), msg.c_str());

			return;
		}
	}
 
	// ����USBKEY�İ汾
	CBankData::GetInstance ()->UpdateUSB (pUsbNode->vid, pUsbNode->pid, pUsbNode->mid, strVersion);


	if (UrlList.size() <= 0)
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_ERROR_PRO_CORE, ERR_DOWNLOAD_URL_NOFOUND_USBKEY, L"MyBankUsbKeyDownloadû�м�⵽�������ӣ�");
	}

	std::list<std::wstring>::iterator it;
	for (it = UrlList.begin (); it != UrlList.end (); it ++) // ������������
	{
		DOWN_LOAD_PARAM_NODE myNode;
		myNode.bCreateThread = true;
		myNode.strHWID = m_strHWID;
		myNode.emKind = emUsbKey; // ����USBKEY
		
		myNode.strSetupPath = m_wstrSetupPath;
		myNode.bSetupDlFile = true;
		myNode.strSaveFile = strCtrlPath;
		myNode.strUrl = (*it);

		// ��ʼ��USBKEY���еĲ���
		myNode.dwUsbKeyParam.nMid = pUsbNode->mid;
		myNode.dwUsbKeyParam.nPid = pUsbNode->pid;
		myNode.dwUsbKeyParam.nVid = pUsbNode->vid;
		myNode.dwUsbKeyParam.strVersion = pUsbNode->ver;


		pTempThread->CreateDownLoadTask (&myNode);
		
	}

}

// ����һ����������
void CBkInfoDownloadManager::DownloadBankCtrl(LPSTR lpBankID, LPCTSTR lpszUrl, LPCTSTR lpszSaveFile, LPVOID lpPostData, DWORD dwPostDataLength)
{
}

// ����������Ӧ�Ĺ���ָ��
CDownloadManagerThread* CBkInfoDownloadManager::FindBankCtrlDLManager(LPSTR lpBankID)
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return NULL;
	
	std::map<std::string, CDownloadManagerThread*>::const_iterator it;

	::EnterCriticalSection(&m_cs);


	it = m_BankManager.find (lpBankID);
	if (m_BankManager.end () == it)
	{
		::LeaveCriticalSection (&m_cs);
		return NULL;
	}

	::LeaveCriticalSection (&m_cs);

	return it->second;
}

// �����������˵ķ��ص�����
bool CBkInfoDownloadManager::ParseBkCtrlListContent(const char* pContent, std::list<std::wstring>& UrlList, std::string& strCheckCode)
{
	ATLASSERT (NULL != *pContent);
	if (NULL == *pContent)
		return false;

	USES_CONVERSION;
	UrlList.clear();

	TiXmlDocument xmlDoc;
	xmlDoc.Parse(pContent); // �������ļ�����

	if (xmlDoc.Error())
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, xmlDoc.Error(), L"TiXmlDocument��ParseBkCtrlListContent");
		return false;
	}

	const TiXmlNode* pRoot = xmlDoc.FirstChild("moneyhub"); // ANSI string 
	if (NULL == pRoot)
		return false;

	const TiXmlNode* pStatus = pRoot->FirstChild("status");
	if (NULL == pStatus)
		return false;

	const TiXmlNode* pStatusId = pStatus->FirstChild("id");
	if (NULL == pStatusId)
		return false;

	const TiXmlElement* pStatusIdElement = pStatusId->ToElement();
	const char* pText = pStatusIdElement->GetText();
	DWORD dwStatusId = pText != NULL ? atoi(pText) : 0; // ID

	//const TiXmlNode* pStatusMsg = pStatus->FirstChild("message");
	//if (NULL != pStatusMsg)
	//{
	//	const TiXmlElement* pStatusMsgElement = pStatusMsg->ToElement();
	//	pText = pStatusMsgElement->GetText();
	//	//m_strStatusMsg = pText != NULL ? AToW(pStatusMsgElement->GetText()) : L""; // message type
	//}

	if (dwStatusId != 300) // ID type
		return false;

	const TiXmlNode* pModules = pRoot->FirstChild("modules");
	if (NULL == pModules)
		return false;

	for (const TiXmlNode *pModule = pModules->FirstChild("module"); pModule != NULL; pModule = pModules->IterateChildren("module", pModule))
	{
		const TiXmlElement* pModuleElement = pModule->ToElement();
		std::string strType = "";
		if (pModuleElement->Attribute("type"))
			strType = pModuleElement->Attribute("type");

		bool bCriticalPack = _stricmp(strType.c_str(), "critical") == 0;

		const TiXmlNode* pModuleName = pModule->FirstChild("name");
		if (NULL == pModuleName)
			continue;

		const TiXmlElement* pModuleNameElement = pModuleName->ToElement();
		//get base64
		pText = pModuleNameElement->Attribute("check");
		strCheckCode = pText; // �õ�У����
		if (!CheckCheckCode (strCheckCode))
			return false;

		pText = pModuleNameElement->GetText();
		std::string strVersionName = pText;
		std::wstring name = pText != NULL ? A2W(pModuleNameElement->GetText()) : L""; // name


		const TiXmlNode* pModulePath = pModule->FirstChild("path");
		if (NULL == pModulePath)
			continue;

		const TiXmlElement* pModulePathElement = pModulePath->ToElement();
		pText = pModulePathElement->GetText();
		std::wstring url = pText != NULL ? A2W(pModulePathElement->GetText()) : L"";

		if (!url.empty())
		{
			// ��ӵ������б���
			UrlList.push_back (url);
		}
	}

	if (UrlList.size() == 0)
	{
		return false;
	}
	return true;
}

bool CBkInfoDownloadManager::ParseUSBListContent(const char* pContent, std::list<std::wstring>& UrlList, std::string& strVersion, std::string& strCheck)
{
	UrlList.clear();
	strVersion.clear ();
	strCheck.clear ();
	ATLASSERT (NULL != pContent);
	if (NULL == pContent)
		return false;

	USES_CONVERSION;

	TiXmlDocument xmlDoc;
	xmlDoc.Parse(pContent); // �������ļ�����

	if (xmlDoc.Error())
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, xmlDoc.Error(), L"TiXmlDocument��ParseUSBListContent");
		return false;
	}

	const TiXmlNode* pRoot = xmlDoc.FirstChild("moneyhub"); // ANSI string 
	if (NULL == pRoot)
		return false;

	const TiXmlNode* pNode = pRoot->FirstChild ("usbinfo");
	if (NULL == pNode)
		return false;

	for (; pNode != NULL; pNode = pRoot->IterateChildren("usbinfo", pNode))
	{
		const TiXmlNode* pUrlNode = pNode->FirstChild ("url");
		if (pUrlNode == NULL)
			continue;

		const TiXmlElement* pUrlEle = pUrlNode->ToElement ();
		const char* pText = pUrlEle->GetText ();
		std::wstring url = pText != NULL ? A2W(pUrlEle->GetText()) : L"";
		UrlList.push_back (url);

		const TiXmlNode* pVerNode = pNode->FirstChild ("ver");
		if (pVerNode == NULL)
			continue;

		const TiXmlElement* pVerEle = pVerNode->ToElement ();
		pText = pVerEle->GetText ();
		strVersion = pText != NULL ? pVerEle->GetText() : "";
		


		const TiXmlNode* pCheckNode = pNode->FirstChild ("checknode");
		if (pCheckNode == NULL)
			continue;

		const TiXmlElement* pCheckEle = pCheckNode->ToElement ();
		pText = pCheckEle->GetText ();
		strCheck = pText != NULL ? pCheckEle->GetText() : "";

		CheckCheckCode(strCheck);
	
	}

	return true;
}

// ��������BankCtrl�ͷ�����ͨѶ��xml�ļ�
bool CBkInfoDownloadManager::ConstructBkCtrlCommunicateXml(LPSTR lpBankID, std::string& info)
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return false;

	LPSTR szXmlInfoPattern = "<?xml version=\"1.0\" encoding=\"utf-8\"?><moneyhub><modules><module><name>";
	std::string allXmlStr = szXmlInfoPattern;
	std::string strBankName = CFavBankOperator::GetBankIDOrBankName (lpBankID);
	allXmlStr += strBankName;
	allXmlStr += "</name></module></modules></moneyhub>";
	
/////////////////////////////////////////////////////////////////////////
	DWORD dwBufLen = strlen(allXmlStr.c_str()) + 32;
	LPSTR lpszXmlInfo = new char[dwBufLen];

	strcpy_s(lpszXmlInfo,dwBufLen,allXmlStr.c_str());

	int dwSize = strlen(lpszXmlInfo) * 2 + 1;
	unsigned char* pszOut = new unsigned char[dwSize];
	base64_encode((LPBYTE)lpszXmlInfo, strlen(lpszXmlInfo), pszOut, &dwSize);
	pszOut[dwSize] = 0;

	
	info = "xml=";
	info += UrlEncode((char *)pszOut);

	delete []pszOut;
	delete []lpszXmlInfo;

	return true;
}

// ��������USB�ͷ���������ͨѶ��xml�ļ�
bool CBkInfoDownloadManager::ReadUSBCommunicateXml(const std::string& strIn, std::string& info)
{
	if (strIn.empty ())
		return false;

	USBRECORD usbRecord;
	memset (&usbRecord, 0, sizeof (USBRECORD));

	std::string strTp = strIn, strPid, strVid, strMid;
	size_t nIndex = 0;
	nIndex = strTp.find (PID_MARK_STRING); // �õ�PID��ʼλ��
	if (nIndex < 0)
		return false;

	strTp = strTp.substr (nIndex + 1 + PID_MARK_STRING.length (), strTp.length ());
	nIndex = strTp.find (VID_MARK_STRING); // �õ�VID����ʼλ��
	if (nIndex < 0)
		return false;
	strPid = strTp.substr (0, nIndex);

	strTp = strTp.substr (nIndex + 1 + VID_MARK_STRING.length (), strTp.length ());
	nIndex = strTp.find (MID_MARK_STRING);
	if (nIndex < 0)
		return false;
	strVid = strTp.substr (0, nIndex);

	strMid = strTp.substr (nIndex + MID_MARK_STRING.length (), strTp.length ());

	if (strPid.empty () || strVid.empty () || strMid.empty ())
		return false;
	


	// ��ȡ�����ݿ�ļ�¼
	CBankData* pBankData = CBankData::GetInstance();
	if (pBankData->GetAUSBRecord (atoi (strVid.c_str ()), atoi(strPid.c_str ()), atoi(strMid.c_str ()), usbRecord))
	{
		info = usbRecord.xml;
		return true;
	}
	else
		return false;
}

std::string CBkInfoDownloadManager::UrlEncode(const std::string& src)   
{   
    static char hex[] = "0123456789ABCDEF";   
    std::string dst;   
  
    for (size_t i = 0; i < src.size(); i++)   
    {   
        unsigned char ch = src[i];   
        if (isalnum(ch))   
        {   
            dst += ch;   
        }   
        else  
            if (src[i] == ' ')   
            {   
                dst += '+';   
            }   
            else  
            {   
                unsigned char c = static_cast<unsigned char>(src[i]);   
                dst += '%';   
                dst += hex[c / 16];   
                dst += hex[c % 16];   
            }   
    }   
    return dst;   
}

bool CBkInfoDownloadManager::CheckBankCtrlXml(LPCTSTR lpPath, std::list<std::wstring>& UrlList, std::string& strCheckCode)
{
	strCheckCode.clear ();

	ATLASSERT (NULL != lpPath);
	if (NULL == lpPath)
		return false;

	HANDLE hFile = CreateFile(lpPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dwLowSize = GetFileSize(hFile, NULL);
	if (dwLowSize == INVALID_FILE_SIZE)
	{
		CloseHandle(hFile);

		return false;
	}

	DWORD dwRead = 0;
	char* pContent = new char[dwLowSize + 1];
	if (!ReadFile(hFile, pContent, dwLowSize, &dwRead, NULL))
	{
		delete[] pContent;
		CloseHandle(hFile);

		return false;
	}

	CloseHandle(hFile);
	*(pContent+dwLowSize) = '\0';


	if (!ParseBkCtrlListContent(pContent, UrlList, strCheckCode)) // �������пؼ�xml�ļ�
	{
		delete[] pContent;
		return false;
	}

	delete[] pContent;

	return true;
}

bool CBkInfoDownloadManager::CheckUsbKeyXml(LPCTSTR lpPath, std::list<std::wstring>& UrlList, std::string& strCheckCode, std::string& strVersion)
{
	UrlList.clear ();
	strCheckCode.clear ();
	strVersion.clear ();

	ATLASSERT (NULL != lpPath);
	if (NULL == lpPath)
		return false;

	HANDLE hFile = CreateFile(lpPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dwLowSize = GetFileSize(hFile, NULL);
	if (dwLowSize == INVALID_FILE_SIZE)
	{
		CloseHandle(hFile);

		return false;
	}

	DWORD dwRead = 0;
	char* pContent = new char[dwLowSize + 1];
	if (!ReadFile(hFile, pContent, dwLowSize, &dwRead, NULL))
	{
		delete[] pContent;
		CloseHandle(hFile);

		return false;
	}

	CloseHandle(hFile);
	*(pContent+dwLowSize) = '\0';

	if (!ParseUSBListContent(pContent, UrlList, strVersion, strCheckCode)) // ��������USBKEYʱ���������ص�xml�ļ�
	{
		delete[] pContent;
		return false;
	}

	delete[] pContent;

	return true;
}

// У����У��
bool CBkInfoDownloadManager::CheckCheckCode(const std::string& strCheck)
{
	unsigned char content[4000]={0};
	int dwReturnSize = 0;

	//for(std::vector<std::string>::size_type i = 0; i < m_checkBase64.size (); i ++) // gao

	base64_decode((unsigned char *)strCheck.c_str(), strCheck.size(),content,&dwReturnSize);	

	*(content + dwReturnSize) = 0;

	if(!BankMdrVerifier::InitCheck((const char *)content,dwReturnSize) )
	{
		return false;
	}

	return true;

}

// �õ����ص���ʱĿ¼
std::wstring CBkInfoDownloadManager::GetTempCachePath()
{
	char* pTempPath = new char[MAX_PATH + 1];
	memset (pTempPath, 0, MAX_PATH + 1);

	// ��Application Data·��
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, pTempPath);

	std::wstring path = CA2W(pTempPath);
	if (path[path.size() - 1] != '\\')
		path += _T("\\");

	path += _T("MoneyHub\\");
	::CreateDirectoryA(CW2A(path.c_str ()), NULL);


	path += _DownLoadPath;
	::CreateDirectoryA(CW2A(path.c_str ()), NULL);

	delete []pTempPath;

	return path;
}

DWORD CBkInfoDownloadManager::CheckThreadProc(LPVOID lpParam)
{
	CBkInfoDownloadManager *pTemp = (CBkInfoDownloadManager*)lpParam;
	while(true)
	{

		// �鿴�����Ƿ����
		
		std::map<std::string, CDownloadManagerThread*>::const_iterator cstIt;

		::EnterCriticalSection(&(pTemp->m_cs));
		for (cstIt = pTemp->m_BankManager.begin (); cstIt != pTemp->m_BankManager.end (); cstIt ++)
		{

			CDownloadManagerThread* pFinish = (*cstIt).second;
			ATLASSERT (NULL != pFinish);
			if (NULL == pFinish)
				continue;

			if (!pFinish->HasDownloadTask ())
				continue;

			bool bUseable = false;
			nsDownStates emStatus = nsNULL;
			int nPercent = 0;
			
			MY_DOWNLOAD_KIND emMyKind = emOther;


			CDownloadAndSetupThread* pDlThread = pFinish->SetPercentAndGetAFinishDownloadThread ((LPSTR)(*cstIt).first.c_str());

			if (NULL != pDlThread)
			{
				emMyKind = pDlThread->GetDownloadKind ();
				if (pDlThread->IsCancled ())
				{
					if (emBkActiveX & emMyKind)
					{
						// ȡ�������ղز�ɾ�����ݿ��еļ�¼
						//CNotifyFavProgress::GetInstance()->CancelFav ((*cstIt).first);
					}
					else if (emBkActiveX  & emUsbKey)
					{
						// ȡ��USBKEY�еļ�¼
						CBankData* pBankData = CBankData::GetInstance ();
						pBankData->DeleteUSB(pDlThread->m_dlUSBParam.nVid, pDlThread->m_dlUSBParam.nPid, pDlThread->m_dlUSBParam.nMid);//�ϴΰ�װ���˲�ɾ��
					}
				}
				else if (pDlThread->IsFinished ())
				{

					if (emBkActiveX & emMyKind)
					{
						pTemp->NotifyCoreBankCtrlDLFinish ((LPSTR)(*cstIt).first.c_str ());
						// �������У�����״̬
						nPercent = 200;
						bUseable = true;
					}
					else
					{
						nPercent = 200;
					}

				}
			}

			if (NULL != CNotifyFavProgress::GetInstance()->m_hwndNotify && NULL != pFinish) // ��ȡ�����ͽ���
			{

				if (nPercent < 200)
					nPercent = pFinish->GetAverageDownLoadProcess ();
			
				std::string strID;
				strID = (*cstIt).first;


				bUseable = pFinish->IsBankCtrlSetup ();
				if (nPercent < 100)
				{
					emStatus = nsDownloading; // ��������
				}
				else if (nPercent == 100)
				{
					emStatus = nsInstalling; // ���ڰ�װ
				}
				
				if(nPercent == 200)
				{
					// ���ز���װ���
					if (emBkActiveX & emMyKind)
						CBankData::GetInstance ()->SaveFav ((*cstIt).first, 200);
					else if(emUsbKey & emMyKind)
						CBankData::GetInstance ()->UpdateUSB (pDlThread->m_dlUSBParam.nVid, pDlThread->m_dlUSBParam.nPid, pDlThread->m_dlUSBParam.nMid, 200);
				}
				// �����USBKEY�Ļ����������ý���

				// fan 
				CNotifyFavProgress::GetInstance()->SetFavProgress((*cstIt).first, nPercent, emStatus, bUseable);

			}

			if (NULL != pDlThread)
			{
				if (pFinish->IsBankCtrlSetup ()) // ���пؼ��Ѿ���װ
				{
					// �µ���USBKEY�����Ѿ���װ���
					if (pDlThread->IsFinished () && emUSBBkActiveX == emMyKind)
					{
						DWORD dwThread = 0;
						// �´���һ���̵߳�����Ϣ
						//CloseHandle (CreateThread (NULL, 0, ShowUSBFinishThreadProc, NULL, 0, &dwThread));
					}
				}
			}

			if (pFinish->AllDownloadThreadExit ())
			{
				//if (pFinish->IsBankCtrlCancled ())
				//	CNotifyFavProgress::GetInstance()->CancelFav ((*cstIt).first);

				if (pFinish->IsBankCtrlSetup ()) // ���пؼ��Ѿ���װ
				{
					// ���ز���װ���
				
					CBankData::GetInstance ()->SaveFav ((*cstIt).first, 200);
					
					// fan 
					CNotifyFavProgress::GetInstance()->SetFavProgress((*cstIt).first, nPercent, emStatus, bUseable);

				}
				pFinish->DeleteAllDownLoadThread ();

			}

		}

		::LeaveCriticalSection (&(pTemp->m_cs));

		// �����ɾͽ��а�װ�����޸Ľ���
		Sleep (1000);
	}
}


void CBkInfoDownloadManager::ReadAcquiesceSetupPath()
{
	TCHAR szPath[MAX_PATH];

    if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
		ATLASSERT (false);
        return;
    }

	m_wstrSetupPath = szPath;
	size_t nIndex = m_wstrSetupPath.rfind ('\\');
	ATLASSERT (nIndex >= 0 && nIndex < m_wstrSetupPath.length ());
	if (nIndex < 0 || nIndex >= m_wstrSetupPath.length ())
		return;

	m_wstrSetupPath = m_wstrSetupPath.substr (0, nIndex);
	if (m_wstrSetupPath.rfind ('\\') != m_wstrSetupPath.length () - 1)
		m_wstrSetupPath += L"\\";

	m_wstrSetupPath += _SetUpSubPath;

}

void CBkInfoDownloadManager::CheckDownloadBreakFile(void)
{
	m_wstrDLTempPath = GetTempCachePath(); // �õ���ʱĿ¼
	CString cstrTp = m_wstrDLTempPath.c_str ();
	cstrTp += "\\*.*";
	WIN32_FIND_DATA findData;
	memset(&findData, 0, sizeof(WIN32_FIND_DATAW));
	HANDLE hFFind = FindFirstFile (cstrTp , &findData);

	std::list<std::wstring> downList;
	if (INVALID_HANDLE_VALUE != hFFind)
	{
		do
		{
			std::wstring strFN = findData.cFileName;
			if (strFN == L"." || strFN == L"..")
				continue;

			if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// �ж�xml�ļ�����ʱ�ļ��ǲ��ǳɶԵ�
				if (!IsXmlBreakFileExistInPair (m_wstrDLTempPath.c_str (), strFN.c_str ()))
				{
					// ���ǵ�ɾ��
					DeleteFile ((m_wstrDLTempPath + strFN).c_str ());
					continue;
				}

				// ��������δ��ɵ�����
				if (strFN.find (L".xml") == strFN.length () - 4)
				{
					strFN = strFN.substr (0, strFN.find (L".xml"));
					downList.push_back (strFN);
					//DownloadBankCtrl (strFN.c_str ());
				}
			}
			
		}
		while (FindNextFile (hFFind, &findData));
	}

	FindClose (hFFind);

	std::list<std::wstring>::const_iterator it;
	for (it = downList.begin (); it != downList.end (); it ++)
	{
		DownloadBankCtrl (CW2A((*it).c_str ()));
	}
}

bool CBkInfoDownloadManager::IsXmlBreakFileExistInPair(LPCTSTR lpPath, LPCTSTR lpFileName)
{
	ATLASSERT (NULL != lpPath);
	if (NULL == lpPath)
		return false;

	std::wstring strPath = lpPath;
	std::wstring strFileName = lpFileName;

	if (strPath.length () < 4 || strPath.empty ())
		return false;

	if (strFileName.find (L".xml") == strFileName.length () - 4) // ������xml�ļ�
	{
		strPath += strFileName;

		std::wstring strTp;
		if (!CDownloadThread::TranslanteToBreakDownloadName (strPath, strTp)) // ת������ʱ�ļ���·��
			return false;
		
		HANDLE hFile = CreateFile (strTp.c_str (), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			CloseHandle (hFile);
			return true;
		}
		else
			return false;
	}
	else if (strFileName.find (L".mh") == strFileName.length () - 3) // ��������ʱ�ļ�
	{

		// �õ���ʱ�ļ���Ӧ��xml�ļ���
		strFileName = strFileName.substr (0, strFileName.length () - 3);
		if (strFileName.find (L".") <= 0)
			return false;

		strFileName = strFileName.substr (strFileName.find (L".") + 1, strFileName.length ());
		strFileName += L".xml";

		// �õ�ȫ·��
		strPath += strFileName;
		
		HANDLE hFile = CreateFile (strPath.c_str (), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			CloseHandle (hFile);
			return true;
		}
		else
			return false;
	}
	else
		return false;


}

// �����Ƿ��Ѿ���װ��
bool CBkInfoDownloadManager::IsSetupAlready(LPSTR lpBankID)
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return false;

	wchar_t message[MSG_BUF_LEN];
	////��֤���пؼ�������
	std::string strBankPath = CW2A (m_wstrSetupPath.c_str ()); 
	
	std::string strBankName = CFavBankOperator::GetBankIDOrBankName (lpBankID);
	strBankPath += strBankName;

	CSecurityCheck temp;
	std::string filePath;
	filePath = strBankPath + "\\" + "bank.mchk" ;

	return (temp.VerifyMoneyHubList(strBankPath.c_str(), filePath.c_str(), message) < 0)? false:true;
}

void CBkInfoDownloadManager::CancleDownload(LPSTR lpBankID) // �û�ȡ������
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return;

	CDownloadManagerThread* pFindManager = FindBankCtrlDLManager(lpBankID);
	if (NULL == pFindManager)
		return;

	pFindManager->CancleAllDownload ();
	
	// ��չ����߳��еļ�¼
	pFindManager->DeleteAllDownLoadThread();
}

void CBkInfoDownloadManager::PauseDownload(LPSTR lpBankID) // �û���ͣ����
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return;

	CDownloadManagerThread* pFindManager = FindBankCtrlDLManager(lpBankID);
	if (NULL == pFindManager)
		return;

	pFindManager->PauseAllDownload ();

}

void CBkInfoDownloadManager::SetCheckFun(IBankDownInterface* pObject)
{
	m_staticpICheckFile = pObject;
}

DWORD WINAPI CBkInfoDownloadManager::DownloadBkUSBThreadProc(LPVOID lpParam)
{
	// �õ��û�û��������ɵ�USBKEY�ؼ�
	CBankData* pBankData = CBankData::GetInstance();
	std::vector<LPUSBRECORD> vec;
	pBankData->GetAllUsb (vec);
	std::vector<LPUSBRECORD>::const_iterator cstItUsb;

	CBkInfoDownloadManager *pTemp = CBkInfoDownloadManager::GetInstance ();
	for (cstItUsb = vec.begin (); cstItUsb != vec.end (); cstItUsb ++)
	{
		LPUSBRECORD lpTempNode = *cstItUsb;
		ATLASSERT (lpTempNode != NULL);
		if (NULL == lpTempNode)
			continue;

		// ���û�гɹ���װ
		if (lpTempNode->status < 200)
		{
			pTemp->MyBankUsbKeyDownload (lpTempNode);
		}

		// �����ͷ��ڴ�
		delete lpTempNode;
		lpTempNode = NULL;
	}
	return 0;
}

// ����û��������ɵĻ��ƻ����ղ����пؼ�
void CBkInfoDownloadManager::FinishDLBreakFile(void)
{
	std::list<std::string> strFav;
	CBankData* pBankData = CBankData::GetInstance();

	// �õ��û��ղص�δ������ɵ����пؼ�
	pBankData->GetFav (strFav);
	std::list<std::string>::const_iterator cstIt;
	DWORD dwThreadID = 0;
	for (cstIt = strFav.begin (); cstIt != strFav.end (); cstIt ++)
	{
		DWORD dwThreadID = 0;
		char* appid = new char[20];
		memset(appid, 0, 20);
		strcpy_s(appid, 20, (*cstIt).c_str ());

		CloseHandle (CreateThread (NULL, 0, CExternalDispatchImpl::DownloadBkCtrlThreadProc,(LPVOID)appid, 0, &dwThreadID)); 
		//MyBankCtrlDownload ((LPSTR)(*cstIt).c_str ());
	}

//	CloseHandle (CreateThread (NULL, 0, DownloadBkUSBThreadProc, 0, 0, &dwThreadID));
}

CBkInfoDownloadManager* CBkInfoDownloadManager::GetInstance(void)
{
	if (NULL == m_staticInstance)
		m_staticInstance = new CBkInfoDownloadManager ();

	return m_staticInstance;
}

void CBkInfoDownloadManager::SetProgressNotifyHwnd(HWND hNotify) // �������ؽ���֪ͨ�ľ��
{
	CNotifyFavProgress::GetInstance()->SetProgressNotifyHwnd(hNotify);
}

int CBkInfoDownloadManager::MyTwoVersionCompare(std::string& strVer1, std::string& strVer2)
{
	// ���贫����Ǳ�׼���磺1.0.0.0
	std::set<std::string> setStr1, setStr2;
	CFavBankOperator::SeparateStringByChar (setStr1, strVer1, '.');
	CFavBankOperator::SeparateStringByChar (setStr2, strVer2, '.');
	int nVer1 = 0, nVer2 = 0;
	int nPower = 1000;

	std::set<std::string>::const_iterator cstIt1;
	std::set<std::string>::const_iterator cstIt2;
	for (cstIt1 = setStr1.begin (), cstIt2 = setStr2.begin (); cstIt1 != setStr1.end () && cstIt2 != setStr2.end (); cstIt1 ++, cstIt2 ++)
	{
		nVer1 += atoi((*cstIt1).c_str ())*nPower;
		nVer2 += atoi((*cstIt2).c_str ())*nPower;
		if (nPower > 10)
			nPower = nPower / 10;
	}

	if (nVer1 > nVer2)
		return 1;
	else if (nVer1 == nVer2)
		return 0;
	else
		return -1;
}

void CBkInfoDownloadManager::NotifyCoreBankCtrlDLFinish(LPSTR lpBankID)
{
	ATLASSERT (NULL != lpBankID);
	if (NULL == lpBankID)
		return;

	std::string strBkID;
	strBkID = lpBankID;

	CListManager::_()->AddANewFavBank ((LPWSTR)CA2W(strBkID.c_str ()), false);
	CListManager::_()->UpdateHMac ();

	CFavBankOperator* pOper = CListManager::_()->GetFavBankOper();
	ATLASSERT (NULL != pOper);
	if (NULL == pOper)
		return;
	pOper->UpDateFavBankID (strBkID);

	CListManager::_()->CheckCom(strBkID);//���Դ����ö�����ȷ���Ƿ��Ѿ����ؼ���ȷ����

	//int nTp;
	//CFavBankOperator::MyTranslateBetweenBankIDAndInt (strBkID, nTp);// ��������Ĭ��Ϊtrue,��ʾ��bankIDת����int

		// ����֪ͨ����ܣ��û�����ղ�����
	//SendMessage(g_hMainFrame, WM_FAV_BANK_CHANGE, 0, (LPARAM)nTp); // gao ֪ͨ����û��ղ����з����仯
}

IBankDownInterface* CBkInfoDownloadManager::GetInterfaceInstance(void)
{
	return m_staticpICheckFile;
}

DWORD WINAPI CBkInfoDownloadManager::ShowUSBFinishThreadProc(LPVOID lpVoid)
{
	// �ȴ�JSP���½����ŵ����Ի���
	HWND hFrame = ::FindWindow(_T("MONEYHUB_MAINFRAME"), NULL);
	::MessageBox (hFrame, L"USBKEY�����װ����������ʹ�ã�", L"��ʾ", MB_OK | MB_ICONINFORMATION);
	return 0;
}