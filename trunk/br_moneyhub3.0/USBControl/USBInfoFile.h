#include "stdafx.h"
#include "info.h"
#include "../../ThirdParty/tinyxml/tinyxml.h"
#include "../Utils/CloudCheck/CloudFileSelector.h"
#include <list>
using namespace std;
#define USB_INFO_FILE_NAME L"\\Config\\usbkeyinfo.chk"

class CUSBInfoFileManager
{
private:
	CUSBInfoFileManager();
	~CUSBInfoFileManager();

	static CUSBInfoFileManager* m_Instance;
public:
	static CUSBInfoFileManager* GetInstance();
public:
	bool InitUSBManegerList(list<USBKeyInfo>& usbinfo);
private:
	std::string GetFileContent(wstring strPath,bool bCHK);

	void GetHardWareInfo(const TiXmlNode* pHardWare, USBKeyInfo& usbinfo);
	bool GetSoftWareInfo(const TiXmlNode* pSoftWare, USBKeyInfo& usbinfo);

	// ���ñ���ϵͳ�汾��Ϣ
private:
	map<wstring,SystemType> m_sysTypeList;
	void		SplitFileContent(const CStringA& strContent,CCloudFileSelector& cselector);
	// ��¼��ǰ��ϵͳ�汾��Ϣ�ͽṹ��Ϣ
	SystemType	m_tSystem;
	int			m_tArchite;

	void		GetSystemVersion();
	void		GetRegInfo(wstring& szData, USBKeyInfo& usbinfo);
};