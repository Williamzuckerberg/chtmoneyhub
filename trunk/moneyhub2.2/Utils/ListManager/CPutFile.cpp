#include "stdafx.h"
#include "CPutFile.h"
#include "Windows.h"
#include "ResourceManager.h"
#include "../RecordProgram/RecordProgram.h"
CPutFile::CPutFile(const CWebsiteData *pWebsiteData):m_pWebsiteData(pWebsiteData)
{
}


//void CPutFile::AddFile(wstring name, wstring path)
//{
//	
//}
//replaceΪǿ���滻��ǣ�����涨Ŀ¼�µ��ļ�
void CPutFile::CheckFile(wstring name, wstring path, bool replace)
{
	wstring fullPath;
	fullPath = path + L"\\" + name;

	WCHAR expName[MAX_PATH] ={0};
	ExpandEnvironmentStringsW(fullPath.c_str(), expName, MAX_PATH);

	if(::PathFileExistsW(expName) == FALSE)// �ж��ļ��Ƿ����
	{
		wstring file = CResourceManager::_()->GetFilePath(m_pWebsiteData->GetWebsiteType(), m_pWebsiteData->GetID(), name.c_str());

		::CopyFileW(file.c_str(), expName , TRUE);
		
		DWORD re = ::GetLastError();
		if(re == ERROR_ACCESS_DENIED)
		{	
			USES_CONVERSION;
			string appid;
			if(m_pWebsiteData)
			{
				USES_CONVERSION;
				appid = CFavBankOperator::GetBankIDOrBankName(W2A(m_pWebsiteData->GetID()),false);
				CWebsiteData::StartUAC(A2W(appid.c_str()));
			}
		}
	}
	else
	{
		if(replace == true)//ǿ���滻
		{
			wstring file = CResourceManager::_()->GetFilePath(m_pWebsiteData->GetWebsiteType(), m_pWebsiteData->GetID(), name.c_str());

			::CopyFileW(file.c_str(), expName , FALSE);//
			DWORD re = ::GetLastError();
			if(re == ERROR_ACCESS_DENIED)
			{
				USES_CONVERSION;
				string appid;
				if(m_pWebsiteData)
				{
					USES_CONVERSION;
					appid = CFavBankOperator::GetBankIDOrBankName(W2A(m_pWebsiteData->GetID()),false);
					CWebsiteData::StartUAC(A2W(appid.c_str()));
				}
			}
		}
	}
	return;
}
void CPutFile::CheckExe(wstring installname, wstring name, wstring path, bool replace)
{
	return;
}
