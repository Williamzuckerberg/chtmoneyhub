#include "StdAfx.h"
#include "postData.h"
#include "UrlCrack.h"
#include "..//CryptHash/base64.h"


#define MONEYHUB_USERAGENT _T("Moneyhub/2.0")
#define MS_USERAGENT _T("Mozilla/4.0 (compatible; MSIE 6.0;Windows NT 5.1)\r\n")
#define MS_CONTENTTYPE _T("Content-Type: application/x-www-form-urlencoded\r\n")
#define BUFFSIZE 1024

CPostData* CPostData::m_instance = NULL;
std::tstring m_strUrl;
std::tstring m_strSaveFile;

CPostData * CPostData::getInstance()
{
	if(m_instance == NULL)
		m_instance = new CPostData();
	return m_instance;
}


CPostData::CPostData()
{
	USES_CONVERSION;
	m_strHWID = A2CT(GenHWID2().c_str());
}

void CPostData::sendData(IN LPCTSTR lpszUrl, IN LPSTR pData, IN DWORD dwLen)
{
	ATLASSERT(NULL != pData && dwLen > 0);
	if (NULL == pData || dwLen <= 0)
		return;

	// ��ת���ɿ��ֽ�
	int nLength = MultiByteToWideChar( CP_ACP,  0, (char*)pData,  -1, 0, 0);
	if (nLength <= 0)
		return;
	WCHAR *pWchar = new WCHAR[nLength + 1];
	memset(pWchar, 0, sizeof(WCHAR)*(nLength + 1));
	MultiByteToWideChar(CP_ACP, 0, (char*)pData, -1, pWchar, nLength);


	// ת����UTF-8�µı���
	nLength = WideCharToMultiByte(CP_UTF8, 0, pWchar, -1, NULL, 0, NULL, NULL);
	if (nLength <= 0)
		return;
	char* pStrData = new char[nLength + 1];
	memset(pStrData, 0, sizeof(char)*(nLength + 1));
	::WideCharToMultiByte(CP_UTF8, 0, pWchar, nLength, pStrData, nLength, NULL,FALSE);

	delete []pWchar;

	std::string allXmlStr = pStrData;
	delete []pStrData;

	DWORD dwBufLen = strlen(allXmlStr.c_str()) + 32;
	LPSTR lpszXmlInfo = new char[dwBufLen];
	
	strcpy_s(lpszXmlInfo,dwBufLen,allXmlStr.c_str());

	int dwSize = strlen(lpszXmlInfo) * 2 + 1;
	unsigned char* pszOut = new unsigned char[dwSize];
	base64_encode((LPBYTE)lpszXmlInfo, strlen(lpszXmlInfo), pszOut, &dwSize);
	pszOut[dwSize] = 0;


	std::string info = "xml=";
	info += UrlEncode((char *)pszOut);

	delete []pszOut;
	delete []lpszXmlInfo;


	PostData(lpszUrl,(LPVOID)info.c_str(),info.size() );
}


std::string CPostData::UrlEncode(const std::string& src)   
{   
	static char hex[] = "0123456789ABCDEF";   
	std::string dst;   

	for (size_t i = 0; i < src.size(); i++)   
	{   
		unsigned char ch = src[i];   
		if (isalnum(ch))   
		{   
			dst += ch;   
		}   
		else  
			if (src[i] == ' ')   
			{   
				dst += '+';   
			}   
			else  
			{   
				unsigned char c = static_cast<unsigned char>(src[i]);   
				dst += '%';   
				dst += hex[c / 16];   
				dst += hex[c % 16];   
			}   
	}   
	return dst;   
}  

int CPostData::PostData(IN LPCTSTR lpszUrl, IN LPVOID lpPostData, IN DWORD dwPostDataLength, IN LPCTSTR lpszSaveFile /*, CUpdateMgr* pUpdateMgr*/)
{
	m_strUrl = lpszUrl;//

	ATLASSERT(dwPostDataLength && lpPostData);
	m_lpPostData = lpPostData;
	m_dwPostDataLength = dwPostDataLength;

	//m_strSaveFile = lpszSaveFile;

	m_hInetSession = NULL;
	m_hInetConnection = NULL;
	m_hInetFile = NULL;
	m_hSaveFile = INVALID_HANDLE_VALUE;

	m_ui64FileSize = 0;
	m_ui64TotalRead = 0;

	return TransferDataPost();
}

void CPostData::CloseHandles()
{
	if (m_hInetConnection)
	{
		InternetCloseHandle(m_hInetConnection);
		m_hInetConnection = NULL;
	}

	if (m_hInetFile)
	{
		InternetCloseHandle(m_hInetFile);
		m_hInetFile = NULL;
	}

	if (m_hInetSession)
	{
		InternetCloseHandle(m_hInetSession);
		m_hInetSession = NULL;
	}

	if (m_hSaveFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hSaveFile);
		m_hSaveFile = INVALID_HANDLE_VALUE;
	}
}
int CPostData::TransferDataPost()
{
	CUrlCrack url;
	if (!url.Crack(m_strUrl.c_str()))
		return ERR_URLCRACKERROR;

	m_hInetSession = ::InternetOpen(MONEYHUB_USERAGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (m_hInetSession == NULL)
		return ERR_NETWORKERROR;


	DWORD dwTimeOut = 60000;
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONTROL_SEND_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_SEND_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);

	m_hInetConnection = ::InternetConnect(m_hInetSession, url.GetHostName(), url.GetPort(), NULL, NULL, INTERNET_SERVICE_HTTP, 0, (DWORD)this);
	if (m_hInetConnection == NULL)
	{
		CloseHandles();
		return ERR_NETWORKERROR;
	}


	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*"); 
	ppszAcceptTypes[1] = NULL;

	m_hInetFile = HttpOpenRequest(m_hInetConnection, _T("POST"), url.GetPath(), NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, (DWORD)this);
	if (m_hInetFile == NULL)
	{
		CloseHandles();
		return ERR_NETWORKERROR;
	}


	HttpAddRequestHeaders(m_hInetFile, _T("Content-Type: application/x-www-form-urlencoded\r\n"), -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE); 
	HttpAddRequestHeaders(m_hInetFile, _T("User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; .NET CLR 1.1.4322; .NET4.0C; .NET4.0E)\r\n"), -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE); 

	TCHAR szHeaders[1024];
	_stprintf_s(szHeaders, _countof(szHeaders), _T("MoneyhubUID: %s\r\n"), m_strHWID.c_str());
	HttpAddRequestHeaders(m_hInetFile, szHeaders, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE); 

	BOOL bSend = ::HttpSendRequest(m_hInetFile, NULL, 0, m_lpPostData, m_dwPostDataLength);
	if (!bSend)
	{
		CloseHandles();
		return ERR_NETWORKERROR;
	}

	CloseHandles();

	return ERR_SUCCESS;
}


int CPostData::TBigDataPost()
{
	//CString sTraceBuffer;
	CUrlCrack url;
	if (!url.Crack(m_strUrl.c_str()))
		return ERR_URLCRACKERROR;

	m_hInetSession = ::InternetOpen(MS_USERAGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (m_hInetSession == NULL)
		return ERR_NETWORKERROR;

	DWORD dwTimeOut = 360000;
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONTROL_SEND_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_SEND_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);
	InternetSetOptionEx(m_hInetSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(DWORD), 0);

	m_hInetConnection = ::InternetConnect(m_hInetSession, url.GetHostName(), url.GetPort(), NULL, NULL, INTERNET_SERVICE_HTTP, 0, (DWORD)this);
	if (m_hInetConnection == NULL)
	{
		CloseHandles();
		return  ERR_NETWORKERROR;
	}

	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*"); 
	ppszAcceptTypes[1] = NULL;

	DWORD dwFlags = 0;

	if( url.GetPort() == 443 )
	{
		dwFlags =	INTERNET_FLAG_NO_CACHE_WRITE |
					INTERNET_FLAG_KEEP_CONNECTION |
					INTERNET_FLAG_PRAGMA_NOCACHE |
					INTERNET_FLAG_SECURE |
					INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
					INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
	}
	else if( url.GetPort() == 80 )
	{
		//for HTTP
		dwFlags =	INTERNET_FLAG_NO_CACHE_WRITE |
					INTERNET_FLAG_KEEP_CONNECTION |
					INTERNET_FLAG_PRAGMA_NOCACHE ;
	}

	m_hInetFile = HttpOpenRequest(m_hInetConnection, _T("POST"), url.GetPath(), NULL, NULL, ppszAcceptTypes, dwFlags, (DWORD)this);
	if (m_hInetFile == NULL)
	{
		CloseHandles();
		return  ERR_NETWORKERROR;
	}
	return ERR_SUCCESS;
}


int CPostData::PostIniBigData(IN LPCTSTR lpszUrl )
{
	m_strUrl = lpszUrl;//

	//ATLASSERT(dwPostDataLength && lpPostData);
	m_hInetSession = NULL;
	m_hInetConnection = NULL;
	m_hInetFile = NULL;
	m_hSaveFile = INVALID_HANDLE_VALUE;

	m_ui64FileSize = 0;
	m_ui64TotalRead = 0;

	return TBigDataPost();
}


BOOL CPostData::UseHttpSendReqEx( std::string& lpost)
{
	long lgg;
	lgg = lpost.size();

	USES_CONVERSION;
	INTERNET_BUFFERS BufferIn;
	DWORD dwBytesWritten;

	BOOL bRet;

	TCHAR head[1024];        
	std::wstring strt = MS_CONTENTTYPE;
	std::wstring strt1 = CA2W(CSNManager::GetInstance()->GetSN().c_str());
	_stprintf_s(head, _countof(head), _T("%s\r\nSN: %s;\r\nMoneyhubUID: %s;\r\n"), strt.c_str() ,strt1.c_str() ,m_strHWID.c_str());
	//TCHAR head[]= strt.c_str(); 

	BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS ); // Must be set or error will occur
    BufferIn.Next = NULL; 
    BufferIn.lpcszHeader = head;
    BufferIn.dwHeadersLength = 0;
    BufferIn.dwHeadersTotal = sizeof(head);
    BufferIn.lpvBuffer = (LPSTR)lpost.c_str();                
    BufferIn.dwBufferLength = 0;
    BufferIn.dwBufferTotal = lgg; // This is the only member used other than dwStructSize
    BufferIn.dwOffsetLow = 0;
    BufferIn.dwOffsetHigh = 0;

    if(!HttpSendRequestEx( m_hInetFile, &BufferIn, NULL, 0, 2))
    {
		CloseHandles();
        printf( "Error on HttpSendRequestEx %d\n",GetLastError() );
        return FALSE;
    }

	bRet=TRUE;
	bRet = InternetWriteFile( m_hInetFile, (LPSTR)lpost.c_str(), lgg, &dwBytesWritten);

	if(!bRet)
	{
     	CloseHandles();
        printf( "\nError on InternetWriteFile %lu\n",GetLastError() );
        return FALSE;
    }

	bRet = HttpEndRequest(m_hInetFile, NULL, 0, 0);
    if(!bRet)
    {
    	CloseHandles();
        printf( "Error on HttpEndRequest %lu \n", GetLastError());
        return FALSE;
    }

		char pcBuffer[BUFFSIZE];
		DWORD dwBytesRead;
	    LPSTR	lpszData1;
		lpszData1 = new char[1024*1024];
		lpszData1[0]='\0';

		//printf("\nThe following was returned by the server:\n");
		do
		{	dwBytesRead=0;
			if(InternetReadFile(m_hInetFile, pcBuffer, BUFFSIZE-1, &dwBytesRead))
			{
				pcBuffer[dwBytesRead]=0x00; // Null-terminate buffer
        		strcat(lpszData1,pcBuffer);
				//printf("%s", pcBuffer);
			}
			else
                return FALSE;
				//lpszData1 ="";
				//printf("\nInternetReadFile failed");
		}while(dwBytesRead>0);
		//printf("\n");
		lpost = "";
		lpost = CW2A(UTF8ToUnicode(lpszData1).c_str());
	    delete []lpszData1;

	CloseHandles();
	//return  ERR_SUCCESS;
	return TRUE;
}


std::wstring CPostData::UTF8ToUnicode(char* UTF8)
{
	DWORD dwUnicodeLen;        //ת����Unicode�ĳ���
	TCHAR *pwText;            //����Unicode��ָ��
	//std::string strUnicode;        //����ֵ

	//���ת����ĳ��ȣ��������ڴ�
	dwUnicodeLen = MultiByteToWideChar(CP_UTF8,0,UTF8,-1,NULL,0);
	pwText = new TCHAR[dwUnicodeLen];
	if (!pwText)
	{
		std::wstring strUnicode1;
		return  strUnicode1;
	}

	//תΪUnicode
	MultiByteToWideChar(CP_UTF8,0,UTF8,-1,pwText,dwUnicodeLen);

	//תΪCString
	//strUnicode.Format(_T("%s"),pwText);
	std::wstring strUnicode(pwText);

	//����ڴ�
	delete []pwText;

	//����ת���õ�Unicode�ִ�
	return strUnicode;
}
