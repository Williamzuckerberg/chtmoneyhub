/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  UserBehavior.cpp
 *      ˵����  ������Ϣ���ʵ���ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.27	���ź�ͨ	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#include "stdafx.h"
#include "UserBehavior.h"
#include "FileVersionInfo.h"
#include "ConvertBase.h"
#include "..\PostData\postData.h"
#include "CollectInstallInfo.h"
#include "../sn/SNManager.h"
#include "../Config/HostConfig.h"
#include "..\VersionManager\versionManager.h"
/*
<UserBehavior type="(Startup|Exit|Navigate)">
<Startup>Desktop|PopupWin|Browser</Startup>
<Version></Version>
<URL></URL>
</UserBehavior>
*/
// Ҫ���͵ķ�����ַ��Ϣ
#define UB_URL _T("data.feedback.php")

CUserBehavior _guBehavior;

CUserBehavior* CUserBehavior::GetInstance()
{
	return &_guBehavior;
}
// ��ʼ�����������л����moneyhub�İ汾��Ϣ
CUserBehavior::CUserBehavior()
{
	TCHAR szPath[1024];
	::GetModuleFileName(NULL, szPath, _countof(szPath));
	*(_tcsrchr(szPath, '\\') + 1) = 0;
	_tcscat_s(szPath, _T("\\MoneyHub.exe"));

	CFileVersionInfo vinfo;
	if (vinfo.Create(szPath))
	{
		std::wstring strFileVersion = vinfo.GetFileVersion();
		replace(strFileVersion.begin(), strFileVersion.end(), ',', '.');
		strFileVersion.erase(remove(strFileVersion.begin(), strFileVersion.end(), ' '), strFileVersion.end());
		m_strMoneyVersion = std::string(CT2A(strFileVersion.c_str()));
	}
	InitializeCriticalSection(&m_cs);

	m_url = CHostContainer::GetInstance()->GetHostName(kFeedback) + UB_URL;
	//����������һ���߳��������ͨ�ţ������п���ʱ���ܳ���sendrequest��ʱ�䳬ʱ����
	m_hThread = NULL;

}
void CUserBehavior::BeginFeedBack()
{
	DWORD dw;
	m_bClose = true;
	m_hThread = CreateThread(NULL, 0, _threadFeedBackToServer, this, 0, &dw);
}
void CUserBehavior::CloseFeedBack()
{
	if(m_hThread != NULL)
		::TerminateThread(m_hThread, 5);
	m_bClose = false;
	m_hThread = 0;
}

DWORD WINAPI CUserBehavior::_threadFeedBackToServer(LPVOID lp)
{
	while( 1 )
	{
		if(!_guBehavior.m_bClose)
			break;
		if(!_guBehavior.m_strFeedBackInfo.empty())
		{
			EnterCriticalSection(&_guBehavior.m_cs);
			std::vector<std::string>::const_iterator itBeg = _guBehavior.m_strFeedBackInfo.begin();
			for (; itBeg != _guBehavior.m_strFeedBackInfo.end(); itBeg ++)
			{
				CPostData::getInstance()->sendData(_guBehavior.m_url.c_str(),(LPSTR)(LPCSTR)(itBeg->c_str()), itBeg->size());
			}
			_guBehavior.m_strFeedBackInfo.clear();
			LeaveCriticalSection(&_guBehavior.m_cs);
		}
		else
			Sleep(5000);//5s�Ӳ�ѯһ���Ƿ�������Ӧ�÷��͸�������
	}
	return 0;

}

// ��װ�����ĺ�����ֻ����һ�Σ�ֱ�ӷ���
void CUserBehavior::Action_Install(int i)
{
	CCollectInstallInfo in;
	string xml = in.GetInstallInfo();
	xml = "<moneyhubversion>" + m_strMoneyVersion + "</moneyhubversion>";
	xml += "</data>";

	wstring str = AToW(xml,CP_ACP);
	string sstr = WToA(str,CP_UTF8);

	if(i == 0)
	{
		EnterCriticalSection(&m_cs);
		m_strFeedBackInfo.push_back(xml);
		LeaveCriticalSection(&m_cs);
		return;
	}

	CPostData::getInstance()->sendData(m_url.c_str(), (LPSTR)(LPCSTR)sstr.c_str(), sstr.size());
}
// ж�ط����ĺ���,����Ҫ�ȣ�ֱ�ӷ����Ͷ���
void CUserBehavior::Action_Uninstall(int i)
{
	// Head
	std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data><action>uninstall</action>";
	xml += "<terminalid>" + GenHWID2() + "</terminalid>";
	xml += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";
	
	// Version
	xml += "<moneyhubversion>" + m_strMoneyVersion + "</moneyhubversion>";

	// Tail
	xml += "</data>";

	if(i == 0)
	{
		EnterCriticalSection(&m_cs);
		m_strFeedBackInfo.push_back(xml);
		LeaveCriticalSection(&m_cs);
		return;
	}

	CPostData::getInstance()->sendData(m_url.c_str(), (LPSTR)(LPCSTR)xml.c_str(), xml.size());
}
// �������������з������ĺ��� ��Ҫ��
void CUserBehavior::Action_ProgramStartup(UBStartupStyle ubss)
{
	// Head
	std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data><action>start</action>";
	xml += "<terminalid>" + GenHWID2() + "</terminalid>";
	xml += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";
	
	// Start Style
	std::string strStyle;
	if (ubss == kDesktop) strStyle = "Desktop";
	else if (ubss == kPopupWin) strStyle = "PopupWin";
	else strStyle = "Browser";

	xml += "<starttype>" + strStyle + "</starttype>";

	// Version
	xml += "<moneyhubversion>" + m_strMoneyVersion + "m</moneyhubversion>";

	// Tail
	xml += "</data>";

	wstring str = AToW(xml,CP_ACP);
	string sstr = WToA(str,CP_UTF8);

	EnterCriticalSection(&m_cs);
	m_strFeedBackInfo.push_back(xml);
	LeaveCriticalSection(&m_cs);

	//CPostData::getInstance()->sendData(m_url.c_str(), (LPSTR)(LPCSTR)xml.c_str(), xml.size());
}

// �˳������ĺ��� ��ֻ����һ�Σ����Ե�
void CUserBehavior::Action_ProgramExit()
{
	// Head
	std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data>";
	xml += "<terminalid>" + GenHWID2() + "</terminalid>";
	xml += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";

	xml += "<action>quit</action></data>";

	EnterCriticalSection(&m_cs);
	m_strFeedBackInfo.push_back(xml);
	LeaveCriticalSection(&m_cs);

	//CPostData::getInstance()->sendData(m_url.c_str(), (LPSTR)(LPCSTR)xml.c_str(), xml.size());
}

// ������������ʷ������ĺ�������Ҫ��
void CUserBehavior::Action_ProgramNavigate(const std::string& strUrl)
{
	// Head
	std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data><action>visit</action>";
	xml += "<terminalid>" + GenHWID2() + "</terminalid>";
	xml += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";

	// Navigate Page
	xml += "<visitedurl>" + strUrl + "</visitedurl>";

	// Tail
	xml += "</data>";

	EnterCriticalSection(&m_cs);
	m_strFeedBackInfo.push_back(xml);
	LeaveCriticalSection(&m_cs);
}

// �������������з������ĺ���  
void CUserBehavior::Action_Upgrade(std::string before,std::string end)
{
	std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data><action>upgrade</action>";
	xml += "<terminalid>" + GenHWID2() + "</terminalid>";
	xml += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";

	// �汾��Ϣ
	xml += "<beforeversion>" + before + "m</beforeversion>";
	xml += "<afterversion>" + end + "m</afterversion>";
	// Tail
	xml += "</data>";

	CPostData::getInstance()->sendData(m_url.c_str(), (LPSTR)(LPCSTR)xml.c_str(), xml.size());
}

// ��������������˵�ģ������ ��ѧϰ������
void CUserBehavior::Action_Study(std::string& strfile, const char* hash,VerifyType vtype, VerifyResult vre)
{
	std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data><action>study</action>";
	xml += "<terminalid>" + GenHWID2() + "</terminalid>";
	xml += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";

	// ������Ϣ
	xml += "<moduleinfo>" + strfile + "mo</moduleinfo>";

	string md5(hash);

	xml += "<modulemd5>" + md5 + "</modulemd5>";

	std::string strType;
	if (vtype == kSysModify) strType = "SysVerify";
	else 
		strType = "CloudVerify";

	xml += "<valichannel>" + strType + "</valichannel>";

	string strResult;
	if(vre == kAllow) strResult = "Allow";
	else
		strResult = "Deny";

	xml += "<valiresult>" + strResult + "</valiresult>";
	// Tail
	xml += "</data>";

	wstring str = AToW(xml,CP_ACP);
	string sstr = WToA(str,CP_UTF8);

	EnterCriticalSection(&m_cs);
	m_strFeedBackInfo.push_back(xml);
	LeaveCriticalSection(&m_cs);

}

// ������
void CUserBehavior::Action_SendErrorInfo(const std::string& strErrCode, const std::string& strErr)
{	
	std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><data><action>filter</action>";
	xml += "<terminalid>" + GenHWID2() + "</terminalid>";
	xml += "<SN>" + CSNManager::GetInstance()->GetSN() +"</SN>";

	// ������Ϣ
	xml += "<errorinfo>ErrorCode:" + strErrCode + ";"  + strErr + "Version:" + m_strMoneyVersion + ";" + "</errorinfo>";

	// Tail
	xml += "</data>";

	EnterCriticalSection(&m_cs);
	m_strFeedBackInfo.push_back(xml);
	LeaveCriticalSection(&m_cs);

	//CPostData::getInstance()->sendData(m_url.c_str(), (LPSTR)(LPCSTR)xml.c_str(), xml.size());
}

// ���������������շ�����Ϣ ֣�� 2011.2.12 15:23 Begin
CUserBehavior::~CUserBehavior()
{
	if(m_hThread != NULL)
		::TerminateThread(m_hThread, 5);
	m_hThread = 0;
	DeleteCriticalSection(&m_cs);
}
// ���������������շ�����Ϣ ֣�� 2011.2.12 15:23 End

void CUserBehavior::Action_SendDataToServerWhenExit(void) // �˳�ʱ�������ݵ�������
{
	/*if(!m_strFeedBackInfo.empty())
	{
		std::vector<std::string>::const_iterator itBeg = m_strFeedBackInfo.begin();
		for (; itBeg!=m_strFeedBackInfo.end(); ++itBeg)
		{
			CPostData::getInstance()->sendData(m_url.c_str(),(LPSTR)(LPCSTR)(itBeg->c_str()),itBeg->size());
		}

		m_strFeedBackInfo.clear();
	}*/
}