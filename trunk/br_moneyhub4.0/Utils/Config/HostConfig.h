#pragma once
#include <string>
#include <map>
#include <vector>
#include <list>
#include "../../ThirdParty/tinyxml/tinyxml.h"
using namespace std;

enum webconfig
{
	kBeginTag, // ��ʼ��־

	kJsWeb = -1, //js�õ�����
	kWeb = 0,//��ҳ��ַ
	kHelp = 1,//����ҳ��ַ
	kFeedback = 2,//�û�����ҳ��ַ
	kUninstall = 3,//ж�ط���ҳ��ַ	
	kDownloadMode = 4, //��������ģʽ
	kPDownloadInstall = 5,//���ؿؼ�
	kPUpgrade = 6,//����
	kPUkey = 7, // usbkey����ļ�
	kPGetFile = 8, // �ϴ��ļ�������
	kPDataFeedback = 9, // ���ݷ���
	kPAutoLogon = 10, // �Զ���½
	kPManuLogon = 11, // �ֶ���½
	kPBeforeRegistration = 12, // ע��ǰУ��
	kPSendMailVerify = 13, // �ط���֤�ʼ�
	kPRegistration = 14, // ע��
	kPGetList = 15, // ��ȡ�����б�
	kPGetDownloadData = 16, // ��������
	kPGetUploadData = 17, // �ϴ�����
	kPChangeMail = 18, // �޸�����
	kPChangePassword = 19, // �޸�����
	kPMailVerify = 20, // ��֤����
	kPUserUnlock = 21, //����û�����״̬
	kPUserServerTime = 22, // ��ȡϵͳʱ��
	kPSendFindMail = 23, // ����OPT��ָ��������
	kPCheckOPT = 24, // У��OPT
	kPInitPassword = 25, // ��������
	kExchangeRate, //���»���

	kEndTag // ������־
};

typedef   std::vector<std::wstring>  VECTORNPBNAME;

class CHostContainer
{
private:
	CHostContainer();
	~CHostContainer();

	static CHostContainer* m_Instance;
public:
	static CHostContainer* GetInstance();

public:
	// ��ʼ����ȡconfig�е�web�ļ�
	void Init(VECTORNPBNAME *pVvecNPB = NULL);
	// ���ݲ��������ַ�ĺ���
	wstring GetHostName(webconfig host);

	bool IsUrlInUrlError(wstring url);

private:
	map<webconfig,wstring> m_host;
	std::list<std::wstring> m_urlError;

	bool ReadUrlData(const TiXmlNode *pErrorHtml);

	std::string GetFileContent(wstring strPath,bool bCHK);
	bool GetAllHostName(VECTORNPBNAME *pVvecNPB = NULL);
};