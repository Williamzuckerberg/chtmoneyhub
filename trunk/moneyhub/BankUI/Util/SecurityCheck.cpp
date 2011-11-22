#include "stdafx.h"
#include "SecurityCheck.h"
#include "../Include/Util.h"
#include "../../Encryption/SHA1/sha.h"
#include "../../Encryption/CHKFile/CHK.h"
#include "ConvertBase.h"
#include "../../Security/BankLoader/BankLoader.h"
#include "../../Security/BankLoader/VerifyCache.h"
#include "../../Security/Authentication/encryption/md5.h"
#include "../../Security/Authentication/ModuleVerifier/export.h"
#include "SysListReader.h"

#include <string>
#include <set>

#include "../ThirdParty/RunLog/Runlog.h"
#include "../ThirdParty/RunLog/LogConst.h"

#include "../Util/DriverCommunicator.h"
#include "../UIControl/SecuCheckDlg.h"
#include "../UIControl/CoolMessageBox.h"

#include "../Utils/ListManager/ListManager.h"
#include "../Utils/FavBankOperator/FavBankOperator.h"
#include "../Utils/UserBehavior/UserBehavior.h"


#include <map>
using namespace std;

CSecurityCheck _SecuCheck;
int CSecurityCheck::ProcessPos = 0;
int CSecurityCheck::ProcessHelp = 0;
bool IsInstall = false;

CSecurityCache g_WhiteCache;
CSecurityCache g_BlackCache;

CSecurityCheck::CSecurityCheck()
: m_fnCheckEventFunc(NULL), m_lpData(NULL)
{
}

void CSecurityCheck::Start(int bCheckType)
{
	DWORD dw;
	if (bCheckType == 0)
	{
		IsInstall = true;
		CloseHandle(CreateThread(NULL, 0, _threadInstallCheck, this, 0, &dw));//��װʱ�ļ��
	}
	else if(bCheckType == 1)
	{
		IsInstall = false;
		CloseHandle(CreateThread(NULL, 0, _threadCheck, this, 0, &dw));// 2���Ӽ��
	}
}
//////////////////////////////////////////////////////////////////////////
//

bool CSecurityCheck::CheckSelfModules()
{
	USES_CONVERSION;

	std::string strModulePath = CT2A(::GetModulePath());
	std::string strCHK = strModulePath + "\\Authen.mchk";
	wchar_t message[MSG_BUF_LEN];

	//Event(CHK_SECURITYCHECK, 5, NULL);

	int ret = VerifySelfModules(strCHK.c_str(), message);
	if (ret < 0)
	{

		//authen������
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_ERROR_ID_CHECK4, MY_ERROR_DESCRIPT_CHECK4);

		Event(CHK_SECURITYCHECK, (DWORD)ret, message, strChkAuthenErr.c_str());
		return false;
	}

	//Event(CHK_SECURITYCHECK, 10, NULL);

	return true;
}

int CSecurityCheck::VerifySelfModules(const char* lpCHKFileName, wchar_t *message)
{
	// ���ļ�
	HANDLE hFile = CreateFileA(lpCHKFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// ��ȡ�ļ�����
		DWORD dwLength = GetFileSize(hFile, NULL);
		unsigned char* lpBuffer = new unsigned char[dwLength + 1];

		if (lpBuffer == NULL)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		DWORD dwRead = 0;
		if (!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"������ģ�������ʧ��");
			return -3002;
		}
		CloseHandle(hFile);

		unsigned char* content = new unsigned char[dwRead];

		if (content == NULL)
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		int contentLength = unPackCHK(lpBuffer, dwRead, content);

		delete []lpBuffer;

		if (contentLength < 0)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
			return -3003;
		}

		delete []content;
		return 0;

	}
	else
	{
		wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
		return -3000;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// by adam
int CSecurityCheck::CheckMoneyHubList(unsigned char *buffer, int length, const wchar_t *path, wchar_t *message)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	char fileName[1024];
	unsigned char md[20];
	SHA_CTX	c;
	int bufferLength;
	unsigned char *ptr;
	int pos;

	USES_CONVERSION;

	ptr=buffer;
	bufferLength=ptr[0]|(ptr[1]<<8)|(ptr[2]<<16)|(ptr[3]<<24);

	if (bufferLength+4!=length)
	{
		wcscpy_s(message, MSG_BUF_LEN, L"������Ϣ��ʽ����");
		return -3010;
	}
	ptr+=4;
	pos=4;
	while (pos<length)
	{
		std::wstring strFile = path;

		bufferLength=ptr[0]|(ptr[1]<<8);
		if (pos+2+bufferLength>length)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"������Ϣ��ʽ����");
			return -3011;
		}
		ptr=ptr+2;
		pos=pos+2;

		memcpy(fileName,ptr,bufferLength);
		fileName[bufferLength]=0;
		ptr+=bufferLength;
		pos+=bufferLength;

		strFile =strFile + A2W(fileName);

		hFile = CreateFile(strFile.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			// ��ȡ�ļ�����
			DWORD dwLength = GetFileSize(hFile, NULL);
			char* lpBuffer = new char[dwLength + 1];

			if(lpBuffer==NULL)
			{
				wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
				return -3001;
			}

			DWORD dwRead = 0;
			if(!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
			{
				delete []lpBuffer;
				swprintf(message, MSG_BUF_LEN, L"���ļ�[%s]ʧ��", strFile.c_str());
				return -3002;
			}

			CloseHandle(hFile);

			SHA1_Init(&c);
			SHA1_Update(&c,(const void *)lpBuffer,dwRead);
			SHA1_Final(md, &c);

			delete []lpBuffer;

			if (memcmp(md,ptr,20)!=0)
			{
				swprintf(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
				return -3012;
			}

			ptr+=20;
			pos+=20;

			//Event(CHK_SECURITYCHECK, 5 * pos / length, strFile.c_str());//ռ��5���ٷֵ�
			//Sleep(50);
		} 
		else 
		{
			swprintf_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
			return -3013;
		}
	}
	if (pos<length)
	{
		wcscpy_s(message, MSG_BUF_LEN, L"������Ϣ��ʽ����");
		return -3014;
	}

	wcscpy_s(message, MSG_BUF_LEN, L"�ɹ�");
	return 0;
}

// by adam
int CSecurityCheck::VerifyMoneyHubList(const char *path, const char* CHKFileName, wchar_t *message)
{
	USES_CONVERSION;

	// ���ļ�
	HANDLE hFile = CreateFileA(CHKFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// ��ȡ�ļ�����
		DWORD dwLength = GetFileSize(hFile, NULL);
		unsigned char* lpBuffer = new unsigned char[dwLength + 1];

		if (lpBuffer == NULL)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		DWORD dwRead = 0;
		if (!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"���ƽ���������������Ϣʧ��");
			return -3002;
		}
		CloseHandle(hFile);

		unsigned char* content = new unsigned char[dwRead];

		if (content == NULL)
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		int contentLength = unPackCHK(lpBuffer, dwRead, content);

		delete []lpBuffer;

		if (contentLength < 0)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
			return -3003;
		}

		int ret = CheckMoneyHubList(content,contentLength, A2W(path),message);

		delete []content;
		return ret;

	}
	else
	{
		wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
		return -3000;
	}
}

//////////////////////////////////////////////////////////////////////////
//
bool CSecurityCheck::CheckBankDataFiles(LPSTR path,wchar_t * message)
{
	std::string dir = path;
	dir += "\\*.*";

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(dir.c_str(), &FindFileData);

	if(INVALID_HANDLE_VALUE == hFind)
		return false;
	do{
		std::string fn = FindFileData.cFileName;

		if ((strcmp(fn.c_str(), ".") != 0) && (strcmp(fn.c_str(), "..") != 0) && (strcmp(fn.c_str(), ".svn") != 0) )
			//&& (_tcsicmp(fn.c_str(), "info.xml") != 0) && (_tcsicmp(fn.c_str(), "syslist.txt") != 0) && (_tcsicmp(fn.c_str(), "CloudCheck.ini") != 0))
		{
			// ��Ŀ¼
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::string subDir = path;
				subDir = subDir + "\\" + fn;
				std::string filePath = path;
				filePath = filePath + "\\" + fn + "\\" + "bank.mchk" ;

				// У��������Ƿ��ղأ�û���ղ�ʱ����
				std::string strBkID = CFavBankOperator::GetBankIDOrBankName(fn, false);
				CFavBankOperator* pOperPointer = CListManager::_()->GetFavBankOper ();
				ATLASSERT (NULL != pOperPointer);
				if (!pOperPointer->IsInFavBankSet (strBkID))
					continue;

				int ret = VerifyMoneyHubList(subDir.c_str(), filePath.c_str(), message);

				if(ret < 0)
				{
					CRecordProgram::GetInstance()->RecordWarnInfo(MY_PRO_NAME, MY_COMMON_ERROR, L"У�������������ݳ���");
					USES_CONVERSION;
					CRecordProgram::GetInstance()->RecordWarnInfo(MY_PRO_NAME, MY_COMMON_ERROR, A2W (filePath.c_str ()));

					FindClose(hFind);
					return false;
				}
			}
			else 
			{
			
			}
		}
	}while (FindNextFileA(hFind, &FindFileData) != 0);

	FindClose(hFind);

	return true;
}

bool CSecurityCheck::CheckSelfDataFiles( bool bCheckBank, bool bThreadCheck)
{
	USES_CONVERSION;

	ProcessPos += 5;
	if(IsInstall)
		Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
	std::string strModulePath = CT2A(::GetModulePath());
	std::string strCHK = strModulePath + "\\MoneyHub.mchk";
	wchar_t message[MSG_BUF_LEN];

	int ret = VerifyMoneyHubList(strModulePath.c_str(), strCHK.c_str(), message);

	ProcessPos += 5;
	if(IsInstall)
		Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);

	if ( ret < 0 )
	{
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_ERROR_ID_CHECK, MY_ERROR_DESCRIPT_CHECK);

		if(!bThreadCheck)
			Event(CHK_SECURITYCHECK, (DWORD)ret, message, strChkMonyHubErr.c_str());
		return false;
	}

	//��֤Html��Config������
	std::vector <std::string> vecDirectoryName;
	vecDirectoryName.push_back("Html");
	vecDirectoryName.push_back("Config");
	int size = vecDirectoryName.size();


	for(int i = 0; i < size; i++)
	{
		std::string strDPath = strModulePath;
		strDPath += "\\" ;
		strDPath += vecDirectoryName[i];

		std::string strDChk = strDPath;
		strDChk += "\\";
		_strlwr_s((LPSTR)vecDirectoryName[i].c_str(), vecDirectoryName[i].size ()+1);
		strDChk += vecDirectoryName[i].c_str();
		strDChk += ".mchk";
		//MessageBoxA(NULL,strDPath.c_str(),strDChk.c_str(),MB_OK);
		
		ret = VerifyMoneyHubList(strDPath.c_str(), strDChk.c_str(), message);
		
		if( ret < 0 )
		{

			CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_ERROR_ID_CHECK2, MY_ERROR_DESCRIPT_CHECK2);

			if(!bThreadCheck)
				Event(CHK_SECURITYCHECK, (DWORD)ret, message, strChkCfDatErr.c_str());
			return false;
		}
	}

	////��֤���пؼ�������
	if( bCheckBank )
	{
		std::string strBankPath =strModulePath ;//getAppdataPath();//; 
		strBankPath += "\\BankInfo\\banks";
		if( false == CheckBankDataFiles((LPSTR)strBankPath.c_str() ,message) ) // ���пؼ�������ʧ�ܣ�����𻵣�ֱ����������
		{

			//moneyhubchk������
			CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_ERROR_ID_CHECK3, MY_ERROR_DESCRIPT_CHECK3);

			 ret = -3002;
			 if(!bThreadCheck)
				Event(CHK_SECURITYCHECK, (DWORD)ret, message, strChkBkDatErr.c_str());
			return false;
		}
		ProcessPos += 10;//20
		 if(IsInstall && !bThreadCheck)
			Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
	}
	////

	return true;
}

//////////////////////////////////////////////////////////////////////////
//

bool CSecurityCheck::CheckSelfUrlList()
{
	std::string strModulePath = CT2A(::GetModulePath());//; getAppdataPath()
	std::string strCHK = strModulePath + "\\BankInfo\\banks\\main\\info.mchk";
	wchar_t message[MSG_BUF_LEN];

	ProcessPos ++;//21
	if(IsInstall)
		Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
	//Event(CHK_SELFMODULES, 11, NULL);
	int ret = VerifyUrlList(strCHK.c_str(), message);
	if (ret < 0)
	{

		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_ERROR_ID_CHECK5, MY_ERROR_DESCRIPT_CHECK5);

		Event(CHK_SELFURLLIST, (DWORD)ret, message, strChkUrlDatErr.c_str());
		return false;
	}

	return true;
}

int CSecurityCheck::VerifyUrlList(const char* lpCHKFileName, wchar_t *message)
{
	// ���ļ�
	HANDLE hFile = CreateFileA(lpCHKFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// ��ȡ�ļ�����
		DWORD dwLength = GetFileSize(hFile, NULL);
		unsigned char* lpBuffer = new unsigned char[dwLength + 1];

		if (lpBuffer == NULL)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		DWORD dwRead = 0;
		if (!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"��URL������ʧ��");
			return -3002;
		}
		CloseHandle(hFile);

		unsigned char* content = new unsigned char[dwRead];

		if (content == NULL)
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		int contentLength = unPackCHK(lpBuffer, dwRead, content);
		delete []lpBuffer;

		if (contentLength < 0)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
			return -3003;
		}

		delete []content;
		return 0;

	}
	else
	{
		wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
		return -3000;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//

bool CSecurityCheck::CheckSelfSysList()
{
	std::string strModulePath = CT2A(::GetModulePath());
	std::string strCHK = strModulePath + "\\Config\\syslist.mchk";
	wchar_t message[MSG_BUF_LEN];

	//Event(CHK_SELFSYSLIST, 0, NULL);

	int ret = VerifySysList(strCHK.c_str(), message);
	if (ret < 0)
	{
		Event(CHK_SELFSYSLIST, (DWORD)ret, message, strChkCfDatErr.c_str());
		return false;
	}

	//Event(CHK_SELFSYSLIST, 100, NULL);

	return true;
}

int CSecurityCheck::VerifyCloudList(const char* lpCHKFileName, wchar_t *message,CCloudFileSelector& cselector)
{
	USES_CONVERSION;

	// ���ļ�
	HANDLE hFile = CreateFileA(lpCHKFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// ��ȡ�ļ�����
		DWORD dwLength = GetFileSize(hFile, NULL);
		unsigned char* lpBuffer = new unsigned char[dwLength + 1];

		if (lpBuffer == NULL)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		DWORD dwRead = 0;
		if (!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"������ʧ��");
			return -3002;
		}
		CloseHandle(hFile);

		unsigned char* content = new unsigned char[dwRead];

		if (content == NULL)
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		int contentLength = unPackCHK(lpBuffer, dwRead, content);

		delete []lpBuffer;

		if (contentLength < 0)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
			return -3003;
		}

		content[contentLength] = '\0';

		CStringA strContent = (char *)content;
		SplitCloudListContent(strContent,cselector);

		delete []content;
		return 0;

	}
	else
	{
		wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
		return -3000;
	}

	return 0;
}

int CSecurityCheck::VerifySysList(const char* lpCHKFileName, wchar_t *message)
{
	USES_CONVERSION;

	// ���ļ�
	HANDLE hFile = CreateFileA(lpCHKFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// ��ȡ�ļ�����
		DWORD dwLength = GetFileSize(hFile, NULL);
		unsigned char* lpBuffer = new unsigned char[dwLength + 1];

		if (lpBuffer == NULL)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		DWORD dwRead = 0;
		if (!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"��ϵͳģ�������ʧ��");
			return -3002;
		}
		CloseHandle(hFile);

		unsigned char* content = new unsigned char[dwRead];

		if (content == NULL)
		{
			delete []lpBuffer;
			wcscpy_s(message, MSG_BUF_LEN, L"�ڴ�ռ���");
			return -3001;
		}

		int contentLength = unPackCHK(lpBuffer, dwRead, content);

		delete []lpBuffer;

		if (contentLength < 0)
		{
			wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
			return -3003;
		}

		//////////////////////////////////////////////////////////////////////////
		// g_sysModuleNameList

		content[contentLength] = '\0';

		CStringA strContent = (char *)content;
		ReadSysList(strContent);

		//////////////////////////////////////////////////////////////////////////

		delete []content;
		return 0;

	}
	else
	{
		wcscpy_s(message, MSG_BUF_LEN, L"���Ĳƽ��ͻ����ѱ��ƻ������������ذ�װ���������Ե��Խ���ȫ��Ĳ���ɨ��");
		return -3000;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//

bool CSecurityCheck::CheckDriverStatus()
{
	//Event(CHK_DRVIMAGEPATH, 0, NULL);

	if (!CDriverLoader::CheckDriverImagePath(false))
	{
		//Driver������
		CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_ERROR_ID_DRIVER, MY_ERROR_DESCRIPT_DRIVER);

		const wchar_t message[] = L"��ȫ�������۸ģ�������ֹ����";
		Event(CHK_DRVIMAGEPATH, (DWORD)-1000, message, strChkDriverErr.c_str());
		return false;
	}

	//Event(CHK_DRVIMAGEPATH, 100, NULL);
	//ProcessPos = 99;//99
	//Event(CHK_SECURITYCHECK, ProcessPos, NULL);
	return true;
}

//////////////////////////////////////////////////////////////////////////
//

bool CSecurityCheck::ExtraCheckGdiplus()
{
	HMODULE hModule = LoadLibraryA("gdiplus.dll");
	if (hModule != NULL)
	{
		TCHAR szModulePath[MAX_PATH + 1];
		GetModuleFileName(hModule, szModulePath, MAX_PATH);
		if (!ModuleVerifier::IsSysModuleVerified(szModulePath))
		{
			//gdiplus������
			CRecordProgram::GetInstance()->RecordWarnInfo(MY_PRO_NAME, MY_ERROR_ID_GDIPLUS, MY_ERROR_DESCRIPT_GDIPLUS);

			//mhMessageBox(NULL, _T("����Windows�����汾�ϵͣ���������Windows���³���������"), SECU_DLG_TITLE, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		}

		FreeLibrary(hModule);
	}

	ProcessPos = 100;//22
	if(IsInstall)
		Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
	return true;
}

//////////////////////////////////////////////////////////////////////////


void CSecurityCheck::SetEventsFunc(CHECKEVENTFUNC pfn, LPVOID lpData)
{
	m_fnCheckEventFunc = pfn;
	m_lpData = lpData;
}

void CSecurityCheck::Event(CheckStateEvent cse, DWORD prog_or_error, const wchar_t* lpszInfo, const char* lpszErrNum)
{
	if (m_fnCheckEventFunc)
		m_fnCheckEventFunc(cse, prog_or_error, lpszInfo, m_lpData);
	else
	{
		// ����г�����Ϣ��������ʾ��
		if (((int)prog_or_error) < 0)
		{
			if(lpszInfo != NULL)
				mhMessageBox(NULL, lpszInfo, SECU_DLG_TITLE, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);

			// ���ͳ�����Ϣ����������
			if (NULL != lpszInfo && NULL != lpszErrNum)
			{
				wstring strErrNum = CA2W(lpszErrNum);				
				CRecordProgram::GetInstance()->FeedbackError(MY_PRO_NAME, MY_COMMON_ERROR, CRecordProgram::GetInstance()->GetRecordInfo(L"������%s-%s", strErrNum.c_str (), lpszInfo));
			}
		}
	}

}
#ifdef SINGLE_PROCESS
#define MY_DEBUG
#endif
DWORD WINAPI CSecurityCheck::_threadCheck(LPVOID lp)
{

	CSecurityCheck* pThis = (CSecurityCheck *)lp;
	CDriverCommunicator cd;

	pThis->Event(CHK_START, 0, NULL, NULL);

#ifndef MY_DEBUG
	if (!pThis->CheckSelfDataFiles(false)) // ���������пؼ����
		return 0;

	if (!pThis->CheckSelfModules())//
		return 0;

	if (!pThis->CheckSelfUrlList())//
		return 0;


	if (!pThis->CheckDriverStatus())
		return 0;

 	if (!pThis->ExtraCheckGdiplus())
 		return 0;
#endif
	pThis->Event(CHK_END, 100, NULL, NULL);


	CRecordProgram::GetInstance()->RecordDebugInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�˳����");

	return 0;
}

DWORD WINAPI CSecurityCheck::_threadInstallCheck(LPVOID lp)
{
	CSecurityCheck* pThis = (CSecurityCheck *)lp;

	pThis->Event(CHK_START, 0, NULL, NULL);

	if (!pThis->CheckSelfDataFiles(false)) // ���������пؼ����
		return 0;

	if (!pThis->CheckSelfModules())//
		return 0;

	if (!pThis->CheckSelfUrlList())//
		return 0;

	g_WhiteCache.SetCacheFileName(L"%Appdata%\\Moneyhub\\WhiteCache.dat");
	g_WhiteCache.Init();
	g_BlackCache.SetCacheFileName(L"%Appdata%\\Moneyhub\\BlackCache.dat");
	g_BlackCache.Init();

	if (!pThis->CheckWhiteListCache())//���ɰ���������
	{
		pThis->Event(CHK_ERROR, 0, NULL, NULL);
		return 0;
	}

	if (!pThis->CheckBlackListCache())//���ɺ���������
	{
		pThis->Event(CHK_ERROR, 0, NULL, NULL);
		return 0;
	}

	g_WhiteCache.Clear();
	g_BlackCache.Clear();

	if (!pThis->CheckDriverStatus())
		return 0;

	if (!pThis->ExtraCheckGdiplus())
		return 0;

	pThis->Event(CHK_END, 0, NULL, NULL);


	CRecordProgram::GetInstance()->RecordDebugInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�˳���װ���");

	return 0;
}

bool CSecurityCheck:: CheckBlackListCache()// ���ɺ�����
{
	CCloudFileSelector cfselector;
	std::string strModulePath = CT2A(::GetModulePath());
	std::string strCHK = strModulePath + "\\Config\\BlackList.mchk";
	wchar_t message[MSG_BUF_LEN];

	int ret = VerifyCloudList(strCHK.c_str(), message,cfselector);
	if (ret < 0)
		return false;
	// ���ԭ�л���
	// �������е��ļ�ֱ������md5ֵ
	set<wstring>* pcfiles = cfselector.GetFiles();

	set<wstring> blacklist;
	set<wstring>::iterator ite = pcfiles->begin();
	for(; ite != pcfiles->end(); ite++)
	{
		SecCachStruct cufile;
		memset(cufile.filename,0,sizeof(cufile.filename));
		wcscpy_s(cufile.filename,MAX_PATH,ite->c_str());

		bool re = g_BlackCache.IsInSecurityCache(cufile);//�õ��Ƿ���Cache�д���
		if(false == re)
		{
			//����ڻ�����,ɾ��,���򲻱�,����˳����˵��˲����ںͷ�MZ���ļ�
			blacklist.insert(cufile.filename);
		}
	}

	if(blacklist.size() > 0)
	{
		set<wstring>::iterator ite = blacklist.begin();
		for(;ite != blacklist.end();ite ++)
		{
			SecCachStruct cufile;
			wcscpy_s(cufile.filename,MAX_PATH,(*ite).c_str());
			if(!g_BlackCache.CalculEigenvalue(cufile))//��������md5ֵ
				continue;

			g_BlackCache.Add(cufile);
		}
	}

	g_BlackCache.GetEigenvalue(g_blackHashList);
	if(g_BlackCache.IsChanged())
		g_BlackCache.Flush();

	return true;
}
DWORD WINAPI CSecurityCheck:: _threadCacheCheck(LPVOID lp)
{
	set<wstring>* pThis = (set<wstring>*)lp;
	if(NULL == lp)
		return 0;

	set<wstring>::iterator ite;
	for(ite = pThis->begin(); ite != pThis->end(); ite++)
	{
		SecCachStruct cufile;
		memset(cufile.filename,0,sizeof(cufile.filename));
		wcscpy_s(cufile.filename,MAX_PATH,ite->c_str());

		bool re = g_WhiteCache.IsInSecurityCache(cufile);//�õ��Ƿ���Cache�д���
		//���ظ�д����������Ҫ����
		if(false == re)
		{
			//����ڻ�����,ɾ��,���򲻱�,����˳����˵��˲����ںͷ�MZ���ļ�
			EnterCriticalSection(&_SecuCheck.m_cs);
			_SecuCheck.m_files.insert(cufile.filename);
			LeaveCriticalSection(&_SecuCheck.m_cs);
		}
		//�ϲ���������ν�ˣ���Ϊ����������ʾ����
		_SecuCheck.m_nowcheck ++;
		if(((int)(_SecuCheck.m_nowcheck * 30 / _SecuCheck.m_listnumber)) > ProcessHelp)
		{
			ProcessHelp ++;
			ProcessPos += 1;//98
			_SecuCheck.Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
		}
	}

	return 0;
}
void CSecurityCheck::ReadSysList(const CStringA& strContent)
{	
	ReadSysList_Plus(strContent);
}

//add by fanzhenxing

void CSecurityCheck::Update(int i)
{
	if((i*3/10) > ProcessHelp)
	{
		ProcessHelp ++;
		ProcessPos += 1;//42
		Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
	}
}

void CSecurityCheck::SplitCloudListContent(const CStringA& strContent,CCloudFileSelector& cselector)
{
	USES_CONVERSION;

	int curPos = 0;
	CStringA resToken = strContent.Tokenize("\r\n", curPos);
	while (resToken != "")
	{
		resToken.Trim();

		if (!resToken.IsEmpty() && resToken.GetAt(0) != ';')
		{
			int nPoundKey = resToken.Find(';');
			if (nPoundKey != -1)
				resToken = resToken.Mid(0, nPoundKey);

			resToken.Replace('/', '\\');

			// (1) Java
			if (resToken.GetAt(0) == '@')
			{
				wstring file = CA2W(resToken.Mid(1));
				cselector.AddWhiteList(1,file);
			}
			// (2) Win7/Vista
			else if (resToken.GetAt(0) == '#')
			{
				wstring file = CA2W(resToken.Mid(1));
				cselector.AddWhiteList(2,file);
			}

			// (3) ����IE����
			else if (resToken.GetAt(0) == '$')
			{
				wstring file = CA2W(resToken.Mid(1));
				cselector.AddWhiteList(3,file);
			}

			else if (resToken.GetAt(0) == '*')
			{
				CStringA restr = resToken.Mid(1);
				int cPos = 0;
				//��õ�ǰ��reg
				wstring reg =  CA2W(restr.Tokenize("+", cPos));
				wstring key = CA2W(restr.Tokenize("+", cPos));
				wstring type = CA2W(restr.Tokenize("+", cPos));
				if(type == L"1")
					cselector.AddRegFolder(reg,key,1);
				else if(type == L"2")
					cselector.AddRegFolder(reg,key,2);
			}

			else if (resToken.GetAt(0) == '&')
			{
				CStringA restr = resToken.Mid(1);
				int cPos = 0;
				//��õ�ǰ��reg
				wstring reg =  CA2W(restr.Tokenize("+", cPos));
				wstring key = CA2W(restr.Tokenize("+", cPos));
				wstring type = CA2W(restr.Tokenize("+", cPos));
				wstring file = CA2W(restr.Tokenize("+",cPos));
				if(type == L"1")
					cselector.AddRegFile(reg,key,file,1);
				else if(type == L"2")
					cselector.AddRegFile(reg,key,file,2);					
			}

			else if ( resToken.GetAt(0) == '!')
			{
				wstring file = CA2W(resToken.Mid(1));

				WCHAR expName[MAX_PATH] ={0};
				ExpandEnvironmentStringsW(file.c_str(), expName, MAX_PATH);
				wstring wtp(expName);
				cselector.AddFolder(expName);
			}

			else if (resToken.GetAt(0) == '^')
			{
				CStringA restr = resToken.Mid(1);
				int cPos = 0;
				//��õ�ǰ��reg
				wstring folder =  CA2W(restr.Tokenize("+", cPos));
				wstring externsion = CA2W(restr.Tokenize("+", cPos));
				cselector.AddExtensionsFile(folder,externsion);					
			}
		}

		resToken = strContent.Tokenize("\r\n", curPos);
	}
}

bool CSecurityCheck::CheckWhiteListCache()
{	
	if(!CheckCache())//�ռ����е��ļ������Ҳ黺��
		return false;

	set<wstring> waitlist;//����ɱ�б�
	//waitlistΪδͨ��΢��ǩ�����б�	
	//ͨ�����б�洢��cloudcheckor�����passfiles��
	CCloudCheckor::GetCloudCheckor()->SetShow((CProcessShow*)this);
	ProcessHelp = 1;
	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"Begin SysModuleVerify");
	CCloudCheckor::GetCloudCheckor()->SysModuleVerify(&m_files,&waitlist);

	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"SysModuleVerify Finish");

	m_files.clear();

	bool cFlag = false;//��¼�Ƿ�������Ʋ�ɱ
	if(waitlist.size() > 0)//��ʱ��������ļ���ͨ����΢����֤������Ҫ�����Ʋ�ɱ��
	{
		//�����Ʋ�ɱ
		CloudCheck(&waitlist,cFlag);
	}
	else
	{
		ProcessPos += 30;//92
		Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
	}

	//���϶��Ѿ���ȫ���ļ����밲ȫ�����С�û�о����Ʋ�ɱ�ķ������ɱ�б���
	set<wstring>* ppsfiles = CCloudCheckor::GetCloudCheckor()->GetPassFiles();
	set<wstring>* pnpsfiles = NULL;
	if(cFlag == true)
		pnpsfiles = CCloudCheckor::GetCloudCheckor()->GetUnPassFiles();

	if((pnpsfiles != NULL) && (pnpsfiles ->size() >0) )
	{
		mhMessageBox(NULL, _T("�����Ʋ�ɱ��ϵͳ�ڿ��ܴ��ڰ�ȫ���գ�����������ȫ��ɱ��ɨ�裡"), SECU_DLG_TITLE, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"ϵͳ���ڰ�ȫ����");

		//��û��ͨ���ķ��������
		set<wstring>::iterator ite;
		for(ite = pnpsfiles->begin(); ite != pnpsfiles->end(); ite++)
		{
			SecCachStruct cufile;
			wcscpy_s(cufile.filename,MAX_PATH,(*ite).c_str());
			if(!g_BlackCache.CalculEigenvalue(cufile))//��������md5ֵ
				continue;

			g_BlackCache.Add(cufile,1);
		}
	}

	// ����hash��
	int k = 0;
	std::set<wstring>::size_type cal = 1; // gao

	// ������΢����֤���Ʋ�ɱ���ļ��ӵ�������
	set<wstring>::iterator ite;
	for(ite = ppsfiles->begin(); ite != ppsfiles->end(); ite++)
	{
		k++;
		SecCachStruct cufile;
		wcscpy_s(cufile.filename,MAX_PATH,(*ite).c_str());
		if(!g_WhiteCache.CalculEigenvalue(cufile))//��������md5ֵ
			continue;

		g_WhiteCache.Add(cufile);

		if((k * 6 / ppsfiles->size()) > cal)
		{
			cal ++;
			ProcessPos += 1;//98
			Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);
		}	
	}

	//�ڰ�װ��ʱ��û�д���ɱ

	ProcessPos = 100;//22
	Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);

	if(g_WhiteCache.IsChanged())
		g_WhiteCache.SetSend(true);
	g_WhiteCache.GetEigenvalue(g_moduleHashList);
	g_WhiteCache.Flush();

	CCloudCheckor::GetCloudCheckor()->Clear();

	return true;
}

bool CSecurityCheck::CheckCache()
{
	if (!CheckSelfSysList())//���ϵͳ�ļ�
		return false;
	//��������ļ�
	CCloudFileSelector cfselector;
	std::string strModulePath = CT2A(::GetModulePath());
	std::string strCHK = strModulePath + "\\Config\\CloudCheck.mchk";
	wchar_t message[MSG_BUF_LEN];

	int ret = VerifyCloudList(strCHK.c_str(), message,cfselector);
	if (ret < 0)
	{
		return false;
	}

	// �����Ʋ�ɱ�ļ�
	cfselector.GetAllFiles();
	set<wstring>* pcfiles = cfselector.GetFiles();

	//�ϲ������ļ�����ʡ���ʱ��
	for(int i = 0; i < (int)g_sysModuleNameList.size(); i++)
	{
		wstring wtp(g_sysModuleNameList[i].GetString());
		transform(wtp.begin(), wtp.end(), wtp.begin(), towupper); //ת����д
		pcfiles->insert(wtp);
	}
	g_sysModuleNameList.clear();//���list��ʡ�ڴ�

	int j = 0;
	m_listnumber =  pcfiles->size();
	if(m_listnumber <= 100)
	{
		mhMessageBox(NULL, _T("����ļ��쳣�������ԣ�"), SECU_DLG_TITLE, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return false;
	}
	m_nowcheck = 0;
	int nCount = pcfiles->size();
	//���̲߳黺�沢�������MZ�ļ�
	set<wstring> threadcheck[3];
	HANDLE chkschd[3];
	set<wstring>::iterator ite;
	for(ite = pcfiles->begin();ite != pcfiles->end();ite ++)
	{
		if(j < (int)(m_listnumber/3))
			threadcheck[0].insert((*ite));
		if((j >= (int)(m_listnumber/3)) && (j < (int)(m_listnumber*2/3)))
			threadcheck[1].insert((*ite));
		if(j >= (int)(m_listnumber*2/3))
			threadcheck[2].insert((*ite));	
		j++;
	}

	InitializeCriticalSection(&m_cs);
	ProcessHelp = 1;
	for(int i = 0; i < 3 ; i ++)
	{
		DWORD dw;
		chkschd[i] = CreateThread(NULL, 0, _threadCacheCheck, (void*)&threadcheck[i], 0, &dw);
	}
	DWORD result = ::WaitForMultipleObjects(3,chkschd,TRUE,600000);

	DeleteCriticalSection(&m_cs);


	if(result == WAIT_TIMEOUT)
	{
		mhMessageBox(NULL, _T("���ʱ���쳣�������ԣ�"), SECU_DLG_TITLE, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return false;
	}
	cfselector.ClearFiles();
	ProcessPos += 1;//41
	Event(CHK_SECURITYCHECK, ProcessPos, NULL, NULL);

	return true;

}
bool CSecurityCheck::CloudCheck(set<wstring>* files,bool& flag)
{
	bool re;
	flag = false;
	//VistaȨ������
	CCloudCheckor::GetCloudCheckor()->SetLog(CRunLog::GetInstance()->GetLog());
	re = CCloudCheckor::GetCloudCheckor()->Initialize();
	if(re != true)
	{
		//mhMessageBox(NULL, _T("��ʼ��ʧ�ܣ��������磬���ܻ�Ӱ��ƽ������"), SECU_DLG_TITLE, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
	}	
	//�Ʋ�ɱ��ʼ���ɹ�
	else
	{
		CCloudCheckor::GetCloudCheckor()->SetFiles(files);
		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"��ʼ�Ʋ�ɱ");
		
		// ��ʼ�Ʋ�ɱ
		ProcessHelp = 1;
		re = CCloudCheckor::GetCloudCheckor()->BeginScanFiles();		

		CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_COMMON_PROCESS, L"�����Ʋ�ɱ");

		if(re != true)
		{
		}
		else
			flag = true;//��¼�������Ʋ�ɱ
		// ж��
		re = CCloudCheckor::GetCloudCheckor()->Uninitialize();
	}

	return re;

}
