#include "stdafx.h"
#include "ConvertBase.h"
#include "Requirement.h"
#include "ResourceManager.h"
#include "../CryptHash/base64.h"


CRequirement::CRequirement(const TiXmlNode *pRequire, const CWebsiteData *pWebsiteData) : m_DriverData(this, pWebsiteData), m_CertFileData(this, pWebsiteData),m_PutFile(pWebsiteData)
{
	for (const TiXmlNode *pFile = pRequire->FirstChild("file"); pFile != NULL; pFile = pRequire->IterateChildren("file", pFile))
	{
		std::wstring strURL = AToW(pFile->ToElement()->Attribute("name"));
		m_FileData.AddFile(strURL.c_str());
	}

	m_strID = AToW(pRequire->ToElement()->Attribute("name"));
	std::wstring strType = AToW(pRequire->ToElement()->Attribute("type"));
	if (strType == _T("activex"))
	{
		std::wstring strReg = AToW(pRequire->ToElement()->Attribute("reg")) + _T(".reg");
		m_RegData.LoadRegData(AToW(CResourceManager::_()->GetFileContent(pWebsiteData->GetWebsiteType(), pWebsiteData->GetID(), strReg.c_str())), pWebsiteData);
		m_eRequireType = Require_ActiveX;
	}
	else if (strType == _T("driver")) // ���� gao
	{
		std::wstring strServiceName = AToW(pRequire->ToElement()->Attribute("service"));
		DWORD start = 1;
		DWORD type = 1;
		if(pRequire->ToElement()->Attribute("rstart") != NULL)
			start = atol(pRequire->ToElement()->Attribute("rstart"));
		if(pRequire->ToElement()->Attribute("rtype") != NULL)
			type = atol(pRequire->ToElement()->Attribute("rtype"));

    	m_DriverData.SetDriverInfo(strServiceName.c_str(), start, type);
		m_eRequireType = Require_Driver;
	}
	else if (strType == _T("cert")) // ֤�� gao
	{
		m_CertFileData.InstallCert();
		m_eRequireType = Require_Cert;
	}
	else if (strType == _T("file"))//�����������Ҫ������������Ҫ���ڹ̶�λ�õ��ļ��ŵ�Ӧ�÷ŵ�λ��
	{
		std::wstring strFileName = AToW(pRequire->ToElement()->Attribute("name"));
		std::wstring strPath = AToW(pRequire->ToElement()->Attribute("path"));
		std::wstring strReplace = AToW(pRequire->ToElement()->Attribute("replace"));
		if(strReplace == L"true")
			m_PutFile.CheckFile(strFileName, strPath, true);
		else
			m_PutFile.CheckFile(strFileName, strPath);
		m_eRequireType = Require_File;
	}
	else if (strType == _T("exe"))//����ȷ��ţ��Ժ���˵
	{
		int type = 0;
		std::wstring strType = AToW(pRequire->ToElement()->Attribute("btype"));
		if(strType == L"reg")
			type = 1;
		else if(strType == L"file")
			type = 2;
		else
			type = 3;
		std::wstring strIsInstallName = AToW(pRequire->ToElement()->Attribute("IsInstallName"));
		std::wstring strFileName = AToW(pRequire->ToElement()->Attribute("name"));

		//std::wstring strInstallPath = AToW(pRequire->ToElement()->Attribute("installPath"));
		m_PutFile.CheckExe(strIsInstallName, strFileName, type);
		m_eRequireType = Require_Exe;
	}
	else if(strType == _T("classid"))
	{
		std::wstring strClassId = AToW(pRequire->ToElement()->Attribute("classid"));
		m_ComInfo.SetClassId(strClassId);
		m_eRequireType = Require_Class;
		
	}
	else
		ATLASSERT(0);
}

bool CRequirement::CheckComInfo()
{
	if(m_eRequireType != Require_Class)
		return true;

	m_ComInfo.CheckCom();

}
void CRequirement::InstallRequirement()
{
	switch (m_eRequireType)
	{
		case Require_Driver:
			m_DriverData.InstallDriver(1,1);
			break;
		case Require_Cert:
			m_CertFileData.InstallCert();
			break;
	}
}
