#pragma once
#include "../../ThirdParty/tinyxml/tinyxml.h"
#include "RegData.h"
#include "DriverData.h"
#include "CertFileData.h"
#include "FileData.h"
#include "CPutFile.h"
#include "ComInfo.h"

enum RequireType
{
	Require_ActiveX,
	Require_Driver,
	Require_Cert,
	Require_File,
	Require_Exe,
	Require_Class
};


class CWebsiteData;

class CRequirement
{

public:

	CRequirement(const TiXmlNode *pRequire, const CWebsiteData *pWebsiteData);

	LPCTSTR GetID() const { return m_strID.c_str(); }
	RequireType GetType() const { return m_eRequireType; }

	void InstallRequirement();

	const CRegData* GetRegData() const { ATLASSERT(m_eRequireType == Require_ActiveX); return &m_RegData; }
	const CDriverData* GetDriverData() const { ATLASSERT(m_eRequireType == Require_Driver); return &m_DriverData; }
	const CFileData* GetFileData() const { return &m_FileData; }

	bool CheckComInfo();
private:

	std::wstring m_strID;
	RequireType m_eRequireType;

	// ÿ��վ�㶼���Լ�������ע������ݡ��������ݡ�֤�����ݺ��ļ�����
	CRegData m_RegData;
	CDriverData m_DriverData;
	CCertFileData m_CertFileData;
	CFileData m_FileData;
	CPutFile m_PutFile;
	CComInfo m_ComInfo;
};
