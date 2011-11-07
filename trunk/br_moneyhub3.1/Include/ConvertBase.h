#pragma once
#include <string>
#include <atlstr.h>

#define AToW AstrToWstr
#define WToA WstrToAstr	

enum CodePage{
	CP_Ansi = CP_ACP,	// ANSI code page
	CP_Mac = CP_MACCP,	// ƻ����˾macϵͳ code page
	CP_Oem = CP_OEMCP,	// OEM code page
	CP_Symbol = CP_SYMBOL,	// Windows 2000/XP:Symbol code page 
	CP_Thread_Ansi = CP_THREAD_ACP, // Windows 2000/XP: Current thread's ANSI code page
	CP_Utf7 = CP_UTF7, // ת��UTF-7
	CP_Utf8 = CP_UTF8  // ת��UTF-8
};

inline std::wstring AstrToWstr(__in LPCSTR szSrc , __in int iSrcCount = -1 , __in DWORD cpCodePage = CP_UTF8 ) ;
inline std::wstring AstrToWstr(const std::string& cstr,__in DWORD cpCodePage = CP_UTF8);
inline LPCWSTR AstrToWstr( __in LPCSTR szSrc, __out LPWSTR wszDst, __in const int iDstCount, __in DWORD cpCodePage = CP_UTF8 );
inline LPCWSTR AstrToWstr( __in LPCSTR szSrc, __in const int iSrcCount , __out LPWSTR wszDst , __in const int iDstCount , __in DWORD cpCodePage = CP_UTF8 ) ;
inline LPCWSTR AstrToWstr( __in LPCSTR szSrc, __out std::wstring& wsDst, __in DWORD cpCodePage = CP_UTF8 );
inline LPCWSTR AstrToWstr( __in LPCSTR szSrc, __in const int iSrcCount , __out std::wstring& wsDst , __in DWORD cpCodePage = CP_UTF8 ) ;


inline std::string WstrToAstr(__in LPCWSTR wszSrc,__in const int iSrcCount = -1 , __in DWORD cpCodePage = CP_UTF8);
inline std::string WstrToAstr(__in const std::wstring& cwstr,__in DWORD cpCodePage = CP_UTF8);
inline LPCSTR WstrToAstr( __in LPCWSTR wszSrc, __out LPSTR pchDest,  __in const int iDstCount, __in DWORD cpCodePage =CP_UTF8 );
inline LPCSTR WstrToAstr( __in LPCWSTR wszSrc, __in const int iSrcCount, __out LPSTR pchDest,  __in const int iDstCount, __in DWORD cpCodePage =CP_UTF8 );
inline LPCSTR WstrToAstr( __in LPCWSTR wszSrc, __out std::string& sDest, __in DWORD cpCodePage = CP_UTF8);
inline LPCSTR WstrToAstr( __in LPCWSTR wszSrc, __in const int iSrcCount, __out std::string& sDest, __in DWORD cpCodePage = CP_UTF8);


// 
// // ANSIC�ַ���ת��ΪUNICODE�ַ���
// std::wstring AstrToWstr(LPCSTR szSrc , __in DWORD cpCodePage )
// {
// 	if ( NULL == szSrc )
// 		return std::wstring() ;
// 
// 	std::wstring wsDst; 
// 	LPCWSTR l_pwstr = AstrToWstr(szSrc,-1,wsDst,cpCodePage);
// 	if ( NULL!=l_pwstr )
// 		return wsDst;
// 	else
// 		return std::wstring();
// }

std::wstring AstrToWstr(__in LPCSTR szSrc , __in int iSrcCount , __in DWORD cpCodePage )
{
	if ( NULL == szSrc )
		return std::wstring() ;

	std::wstring wsDst; 
	LPCWSTR l_pwstr = AstrToWstr(szSrc,iSrcCount,wsDst,cpCodePage);
	if ( NULL!=l_pwstr )
		return wsDst;
	else
		return std::wstring();
}

std::wstring AstrToWstr(const std::string& cstr,__in DWORD cpCodePage)
{
	return AstrToWstr(cstr.c_str(),cstr.size()+1, cpCodePage) ;
}

//************************************************************************
// ����˵��: ���ַ�ת��ΪUNICODE
//
// ��������: AstrToWstr
//
// ����Ȩ��: public
//
// ����ֵ����: LPCWSTR	
//
// ����1: LPCSTR	szSrc   �������,ָ��Ҫת����Դ�ַ���
//
// ����2: LPWSTR	 wszDst   �����������ַ���ת���������ָ����ڴ��У�ָ��Ŀռ�
//								�����㹻�����Դ洢ת������ַ�����
//
// ����3: const DWORD iDstCount  ��������ָ��l_pchDest��ָ����ڴ��Ĵ�С
// 
// ����4: CodePage cpCodePage Ҫת����Դ�ַ���������,Ĭ����CP_UTF8,����ֵ�μ�CodePage
// 
//************************************************************************
LPCWSTR AstrToWstr( __in LPCSTR szSrc, __out LPWSTR wszDst, __in const int iDstCount, __in DWORD cpCodePage )
{
	return AstrToWstr( szSrc , -1 , wszDst , iDstCount , cpCodePage ) ;
}

inline LPCWSTR AstrToWstr( __in LPCSTR szSrc, __in const int iSrcCount , __out LPWSTR wszDst , __in const int iDstCount , __in DWORD cpCodePage ) 
{
	if ( NULL == szSrc )
		return NULL ;

	int iCount = MultiByteToWideChar (cpCodePage, 0, szSrc, iSrcCount, wszDst, iDstCount);
	if ( 0==iCount )
		return NULL ;
	else
		return wszDst;

}

LPCWSTR AstrToWstr( __in LPCSTR szSrc, __out std::wstring& wsDst, __in DWORD cpCodePage)
{
	return AstrToWstr(szSrc , -1 , wsDst , cpCodePage) ;
}

LPCWSTR AstrToWstr( __in LPCSTR szSrc, __in const int iSrcCount , __out std::wstring& wsDst , __in DWORD cpCodePage ) 
{
	if ( NULL == szSrc )
		return NULL ;
	try{
		int srcCount = iSrcCount==-1?strlen(szSrc)+1:iSrcCount ;
		int dstCount = 	::MultiByteToWideChar(cpCodePage,0 , szSrc , srcCount , 0 , 0) ;
		dstCount += 10;
		wchar_t *wBuf = new wchar_t[dstCount] ;
		int iConvert = ::MultiByteToWideChar (cpCodePage, 0, szSrc, srcCount, wBuf, dstCount);
		wsDst = iConvert > 0 ?  wBuf : L"";
		delete[] wBuf;
		return wsDst.c_str ();
	}catch (...){}
	return NULL;
}

//************************************************************************
// ����˵��: UNICODE�ַ�תΪ���ַ�
//
// ��������: WstrToAstr
//
// ����Ȩ��: public
//
// ����ֵ����: LPCSTR	ת������ַ��������ʧ�ܷ��� NULL
//
// ����1: LPCWSTR cpwchSource   �������,ָ��Ҫת����UNICODEԴ�ַ���
//
// ����2: LPSTR	 pchDest   �����������ַ���ת���������ָ����ڴ��У�ָ��Ŀռ�
//								�����㹻�����Դ洢ת������ַ�����
//
// ����3: const DWORD cdwDestSize  ��������ָ��l_pchDest��ָ����ڴ��Ĵ�С
// 
// ����4: CodePage cpCodePage Ҫת����Դ�ַ���������,Ĭ����CP_Ansi,����ֵ�μ�CodePage
// 
//************************************************************************
LPCSTR WstrToAstr( __in LPCWSTR wszSrc, 
	__out LPSTR szDst, 
	__in const int iDstCount, 
	__in DWORD cpCodePage )
{
	return WstrToAstr(wszSrc , -1 , szDst , iDstCount , cpCodePage) ;
}

LPCSTR WstrToAstr( __in LPCWSTR wszSrc, __in const int iSrcCount, __out LPSTR szDst,  __in const int iDstCount, __in DWORD cpCodePage )
{
	if ( NULL == wszSrc )
		return NULL ;

	int iCount = WideCharToMultiByte(cpCodePage,0,wszSrc,iSrcCount,szDst,iDstCount,NULL,FALSE);
	if ( 0==iCount )
		return NULL ;
	else
		return szDst;
}


//************************************************************************
// ����˵��: UNICODE�ַ�תΪ���ַ�
//
// ��������: WstrToAstr
//
// ����Ȩ��: public
//
// ����ֵ����: LPCSTR	
//
// ����1: LPCWSTR cpwchSource   �������,ָ��Ҫת����UNICODEԴ�ַ���
//
// ����2: std::wstring& sDest ����ת����Ľ��
// 
// ����4: CodePage cpCodePage Ҫת����Դ�ַ���������,Ĭ����CP_Ansi,����ֵ�μ�CodePage
// 
//************************************************************************
LPCSTR WstrToAstr( __in LPCWSTR wszSrc, 
	__out std::string& sDest, 
	__in DWORD cpCodePage )
{
	return WstrToAstr(wszSrc , -1 , sDest , cpCodePage) ;
}

LPCSTR WstrToAstr( __in LPCWSTR wszSrc, __in const int iSrcCount, __out std::string& sDest, __in DWORD cpCodePage)
{
	if ( NULL == wszSrc )
		return NULL ;

	// modify by wujian
	// ĳһ������ܵı����������⣬������������Ϊbuffer���Ĳ�������
	// ���Կ���һЩ��Ȼ��ת��󲹸�0
	try{
		int srcCount = iSrcCount==-1?wcslen(wszSrc)+1:iSrcCount ;
		int dstCount = ::WideCharToMultiByte(cpCodePage , 0 , wszSrc , iSrcCount , 0 , 0 , 0 ,0) ;
		dstCount += 10;
		char *buf = new char[dstCount];
		int iConvert = ::WideCharToMultiByte(cpCodePage,0,wszSrc,srcCount,buf,dstCount,NULL,FALSE);
		sDest = iConvert > 0 ? buf : "";
		delete[] buf;
		return sDest.c_str ();
	}catch (...){}
	return NULL;
}


//************************************************************************
// ����˵������UNICODE�ַ���ת��ΪANSI�ַ���
//
// ��������: WStrToAStr
//
// ����Ȩ��: public
//
// ����ֵ����: std::string	����ת�����ANSI�ַ���
//
// ����1: LPCTSTR wszSrc	Ҫת�����ַ���
//
//************************************************************************
// ��UNICODE�ַ���ת��ΪANSI�ַ���

std::string WstrToAstr(__in LPCWSTR wszSrc,__in const int iSrcCount , __in DWORD cpCodePage)
{
	if ( NULL == wszSrc )
		return std::string() ;

	std::string l_str;
	LPCSTR l_pstr = WstrToAstr(wszSrc,iSrcCount,l_str,cpCodePage);
	if ( NULL!=l_pstr )
		return l_str;
	else
		return std::string();
}

std::string WstrToAstr(__in const std::wstring& cwstr,__in DWORD cpCodePage)
{
	return WstrToAstr(cwstr.c_str(),cwstr.size()+1,cpCodePage);
}
