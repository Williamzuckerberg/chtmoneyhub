#include "stdafx.h"
#include "URLList.h"
#include "ConvertBase.h"

CURLList* CURLList::m_Instance = NULL;


CURLList* CURLList::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CURLList();
	return m_Instance;
}

CURLList::CURLList(void)
{
}

CURLList::~CURLList(void)
{
}

bool CURLList::ReadData(const TiXmlNode *pUrlList)
{
	m_WebsiteData.clear();
	WebsiteType eWebsiteType;
	std::string strType = pUrlList->ToElement()->Attribute("name");
	if (strType == "banks")
		eWebsiteType = Website_Bank;
	else
		ATLASSERT(0);

	for (const TiXmlNode *pSite = pUrlList->FirstChild("site"); pSite != NULL; pSite = pUrlList->IterateChildren("site", pSite))
	{
		CWebsiteData *pWebsiteData = new CWebsiteData;
		pWebsiteData->m_strID = AToW(pSite->ToElement()->Attribute("id"));
		pWebsiteData->m_strName = AToW(pSite->ToElement()->Attribute("name"));

		pWebsiteData->m_bNoClose = false;
		if (pSite->ToElement()->Attribute("noclose") != NULL)
		{
			CStringW strNoClose = AToW(pSite->ToElement()->Attribute("noclose")).c_str();
			strNoClose.Trim();
			if (strNoClose.CompareNoCase(L"true") == 0)
				pWebsiteData->m_bNoClose = true;
		}

		pWebsiteData->m_eWebsiteType = eWebsiteType;
		const TiXmlNode *pDomainList = pSite->FirstChild("domains");
		if (pDomainList)
		{

			for (const TiXmlNode *pDomain = pDomainList->FirstChild("domain"); pDomain != NULL; pDomain = pDomainList->IterateChildren("domain", pDomain))
			{
				std::wstring strDomain = AToW(pDomain->ToElement()->Attribute("name"));

				WebDataMap::iterator it = m_WebsiteData.find (strDomain);
				if (it == m_WebsiteData.end()) // ���������
				{						
					m_WebsiteData.insert(std::make_pair(strDomain, pWebsiteData)); // ����
				}

			}

		}
		else
		{
			delete pWebsiteData;
			pWebsiteData = NULL;
		}

	}
	return true;
}
const CWebsiteData* CURLList::GetData(LPCTSTR /*lpszDomain*/, LPCTSTR lpszUrl) const
{
	CString strClearUrl = CListManager::ExtractClearDomain(lpszUrl);
	
	WebDataMap::const_iterator it = m_WebsiteData.begin();
	for (; it != m_WebsiteData.end(); ++it)
	{
		CString strDomain = it->first.c_str();
		CWebsiteData* pWebsiteData = it->second;

		int nPattern = strDomain.Find(_T("/*"));
		if (nPattern == -1)
		{
			if (strDomain.Mid(0, 2) != _T("*."))
			{
				if (strDomain.CompareNoCase(strClearUrl) == 0)
					return pWebsiteData;
			}
			else
			{
				CString strDomain1 = strDomain.Mid(1);
				CString strClearUrl1 = _T(".");
				strClearUrl1 += strClearUrl;

				if (strDomain.CompareNoCase((LPCTSTR)strClearUrl + strClearUrl.GetLength() - strDomain1.GetLength()) == 0)
					return pWebsiteData;
			}
		}
		else 
		{
			strDomain = strDomain.Mid(0, nPattern);
			if (strDomain.Mid(0, 2) != _T("*."))
			{
				if (_tcsnicmp(strDomain, strClearUrl, strDomain.GetLength()) == 0)
					return pWebsiteData;
			}
			else
			{
				CString strDomain1 = strDomain.Mid(1);
				CString strClearUrl1 = _T(".");
				strClearUrl1 += strClearUrl;

				strDomain1.MakeLower();
				strClearUrl1.MakeLower();

				if (strClearUrl1.Find(strDomain1) != -1)
					return pWebsiteData;
			}
		}
	}

	return NULL;
}

