/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  UserBehavior.h
 *      ˵����  ������Ϣ��������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.27	���ź�ͨ	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#pragma once
#include <vector>
#include <string>
// Moneyhub�������ͣ����桢���ݡ��������������
// �����������˵��ֶθ�ʽ
/*
<?xml version="1.0" encoding="UTF-8"?>
<data>
<terminalid>yxTKCfGPO2OxBQ89hEc%2F%2BE5mqqRS6MWT</terminalid>
<SN>1234567890123456</SN>
<action>start</action>
<systemname>Windows Xp</systemname>
<systemversion>sp3 1.1.1.1</systemversion>
<systemlang>CHN</systemlang>
<ieversion>ie8</ieversion>
<antivirusname>360safe</antivirusname>
<cpuname>Intel Core 2 Duo</cpuname>
<productdmi>Dell inc</productdmi>
<productname>optiplex 780</productname>
<memorysize>4GB</memorysize>
<moneyhubversion>2.1.1.1</moneyhubversion>
<beforeversion>2.0.0.0</beforeversion>
<afterversion>2.1.1.1</afterversion>
<starttype>desk_start</starttype>
<visitedurl> http://www.moneyhub.com/boc.php</visitedurl>
<errorinfo>bug......</errorinfo>
<moduleinfo>safe.dell_windowssystem32..</moduleinfo>
<modulemd5>bug......</modulemd5>
<valiresult>permit</valiresult>
<valichannel>cloud safe</valichannel>
</data>*/
enum UBStartupStyle
{
	kDesktop = 0,
	kPopupWin = 1,
	kBrowser = 2,
};
enum VerifyType
{
	kSysModify = 0,
	kCloudModify = 1
};
enum VerifyResult
{
	kAllow = 0,
	kDeny = 1
};

const std::string strIEError = "100001";
const std::string strChkBkDatErr = "100002";
const std::string strChkMonyHubErr = "100003";
const std::string strChkAuthenErr = "100004";
const std::string strChkCfDatErr = "100005";
const std::string strChkUrlDatErr = "100006";
const std::string strChkDriverErr = "100007";


const std::string strCoreRunErr = "200008";
const std::string strFindDangerous = "200009";
const std::string strSelfDataErr = "200010";
const std::string strKeyFileErr = "200011";
const std::string strDriverErr = "200012";

class CUserBehavior
{
public:
	CUserBehavior();
	// ���������������մ�����͵ķ�����Ϣ ֣�� 2011.2.12 15:23 Begin
	~CUserBehavior();
	// ���������������մ�����͵ķ�����Ϣ ֣�� 2011.2.12 15:23 End
	
public:
	// ��װ�����ĺ���
	void Action_Install(int i = 0);
	// ��������ʱ�ķ�������
	// ���������UBStartupStyle ubss ����������
	void Action_ProgramStartup(UBStartupStyle ubss);
	// �����˳�ʱ�ķ�������
	void Action_ProgramExit();
	// �������ʱ�ķ�������
	// ���������std::string& strUrl�������ҳ��
	void Action_ProgramNavigate(const std::string& strUrl);
	// ����ж��ʱ�ķ�������
	void Action_Uninstall(int i = 0);
	// ��������ʱ�ķ�������
	// ���������std::string before������ǰ�İ汾��Ϣ��std::string end��������İ汾��Ϣ
	void Action_Upgrade(std::string before,std::string end);
	
	// ��������������˵�ģ������
	void Action_Study(std::string& strfile, const char* hash, VerifyType vtype, VerifyResult vre);

	// ����������ʹ���
	void Action_SendErrorInfo(const std::string& strErrCode, const std::string& strErr);

	void Action_SendDataToServerWhenExit(void); // �˳�ʱ�������ݵ�������

	static DWORD WINAPI _threadFeedBackToServer(LPVOID lp);// �������з����������߳�

	void BeginFeedBack();
	void CloseFeedBack();

protected:
	CRITICAL_SECTION m_cs;
	HANDLE m_hThread;
	// �洢��ǰmoneyhub�汾��Ϣ
	std::string m_strMoneyVersion;
	
	std::wstring m_url;
public:
	static CUserBehavior* GetInstance();

	// ���m_strFeedBackInfo��Ա��������¼������͵ķ�����Ϣ ֣�� 2011.2.12 15:23 Begin
private:
	std::vector<std::string> m_strFeedBackInfo; 
	// ���m_strFeedBackInfo��Ա��������¼������͵ķ�����Ϣ ֣�� 2011.2.12 15:23 End

};