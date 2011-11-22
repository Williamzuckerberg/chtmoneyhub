#pragma once
#include <string>
#include <map>
#include <vector>
#include <list>
#include "../../ThirdParty/tinyxml/tinyxml.h"
using namespace std;

enum webconfig
{
	kWeb = 0,//վ��
	kDownload = 1,//����
	kFeedback = 2,//����
	kAdv = 3,//���
	kBenefit = 4,//�Ż�ȯ
	kDownloadMode = 5 //��������ģʽ
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