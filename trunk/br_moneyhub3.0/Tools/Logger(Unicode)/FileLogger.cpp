/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  FileLogger.cpp
 *      ˵����  �ļ���־������ʵ���ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#include "FileLogger.h"
#include "LogConst.h"
#include <stdio.h>
//#pragma warning(disable:4996)

CFileLogger::CFileLogger(void)
{
	InitializeCriticalSection(&m_cs);
}

CFileLogger::~CFileLogger(void)
{
	DeleteCriticalSection(&m_cs);
}
/**
* ������־����ļ���С��
*/
void CFileLogger::SetFileSize(long size)
{
	m_filesize = size;
}
/**
* ������־�ļ����ơ�
*/
void CFileLogger::SetFileName(const wstring& filename)
{
	m_file = filename;
}
/**
* ���ļ���д����־��Ϣ�ĺ�����
*/
bool CFileLogger::WriteData(const CLogInfo& info)
{
	wchar_t cinfo[MAX_INFO_LENGTH]={0};
	swprintf(cinfo,MAX_INFO_LENGTH,L"%04d-%02d-%02d %02d:%02d:%02d.%03d\t%d", info.time.wYear, info.time.wMonth, info.time.wDay, 
		info.time.wHour, info.time.wMinute, info.time.wSecond, info.time.wMilliseconds,info.pid);
	wstring sinfo(cinfo);

	sinfo += L"\t" + info.typeinfo + L"\t" + info.info + L"\r\n";


	char *pdata = (char*)sinfo.c_str();
	if(m_filehandle != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(m_filehandle, 0, NULL, FILE_END);
		DWORD wl;
		if(!WriteFile(m_filehandle,pdata,(sinfo.length())*sizeof(wchar_t),&wl,NULL))
			return false;
		return true;
	}

	return false;
}
/**
* ����ļ���С�ĺ�����
*/
bool CFileLogger::CheckFileSize(HANDLE& hFile)
{
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	long dwFileSize = GetFileSize(hFile, NULL);

	if(dwFileSize == 0)
	{
		wchar_t uhead = 0xFEFF;
		DWORD dwLength;
		WriteFile(hFile,&uhead,sizeof(wchar_t),&dwLength,NULL);
	}
	if(dwFileSize >= m_filesize)
	{
		return false;
	}
	else
		return true;
}
/**
* ���캯����
* ��ʼ��������
*/
COneFileStrategyLogger::COneFileStrategyLogger(wstring logdirectory,wstring logfilename,long size)
{
	m_directory = logdirectory + L"\\";
	m_time = 0;
	m_file = logfilename;
	m_filesize = size;
	m_currentfile = m_directory + m_file;
	//Ϊ�˷�ֹ�������Ĵ���,��һ������ͳһʹ�ÿ��ֽ�
	m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
	if(m_filehandle != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(m_filehandle, 0, NULL, FILE_END);
	}
	//_s(&m_plogfile,);//��׷�ӷ�ʽ���ļ�

	else
	{			
		//��һ�δ�ʧ�ܣ���ô����ʱ�ļ�	
		m_currentfile = m_directory + L"~logtemp.dat";
		m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
		if(m_filehandle != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(m_filehandle, 0, NULL, FILE_END);
		}
		else
			return;
	}

	if(!CheckFileSize(m_filehandle))
	{
		CloseHandle(m_filehandle);

		m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//���½���ʽ���ļ�
		if(m_filehandle == INVALID_HANDLE_VALUE)
			return;

		SetFilePointer(m_filehandle, 0, NULL, FILE_END);
		wchar_t uhead = 0xFEFF;
		DWORD dwLength;
		WriteFile(m_filehandle,&uhead,sizeof(wchar_t),&dwLength,NULL);
	}
}

COneFileStrategyLogger::~COneFileStrategyLogger(void)
{
	CloseHandle(m_filehandle);
}

bool COneFileStrategyLogger::Write(const CLogInfo& info)
{
	//ÿд��һ��������
	if(info.type > this->m_level)//�ڱ�ϵͳ�ڼ��д����Ϣ�ļ���
		return false;

	if(m_filehandle == INVALID_HANDLE_VALUE)
		return false;
	//����ʼ�����ļ�ʱ��ʼ��������ֹ�����߳�д��
	EnterCriticalSection(&m_cs);
	if(m_time >= CHECK_TIME)
	{
		if(!CheckFileSize(m_filehandle))
		{
			CloseHandle(m_filehandle);
			m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
					FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
			if(m_filehandle == INVALID_HANDLE_VALUE)
			{
				LeaveCriticalSection(&m_cs);
				return false;
			}

			SetFilePointer(m_filehandle, 0, NULL, FILE_END);
			wchar_t uhead = 0xFEFF;
			DWORD dwLength;
			WriteFile(m_filehandle,&uhead,sizeof(wchar_t),&dwLength,NULL);

		}
		m_time = 0;
	}

	bool res = WriteData(info);
	if(res)
	{
		m_time ++;
	}
	LeaveCriticalSection(&m_cs);

	return res;
}

CTwoFileStrategyLogger::~CTwoFileStrategyLogger(void)
{
	::CloseHandle(m_filehandle);
}

/**
* ���캯����
* ��ʼ��������
*/
CTwoFileStrategyLogger::CTwoFileStrategyLogger(wstring logdirectory,wstring logfilename,long size)
{
	m_directory = logdirectory + L"\\";
	m_current = 0;
	m_time = 0;
	m_file = logfilename;
	m_filesize = size;

	GetFileName();
	//Ϊ�˷�ֹ�������Ĵ������,��һ������ͳһʹ�ÿ��ֽ�
	m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
	if(m_filehandle != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(m_filehandle, 0, NULL, FILE_END);
	}

	else
	{
		//�򿪵�һ���ļ�ʧ�ܣ���ô����ʱ�ļ�
		m_file = L"~logtemp.d";
		GetFileName();
		m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
		if(m_filehandle != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(m_filehandle, 0, NULL, FILE_END);
		}
		else
			return;

	}
	//�������־�ļ��ɹ���������־�ļ���С��������ϴ�С����ô���ø��ļ���������־�ļ�
	if(!CheckFileSize(m_filehandle))
	{
		::CloseHandle(m_filehandle);
		m_current = (m_current > 0) ? 0 : 1;

		GetFileName();
		m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
		if(m_filehandle == INVALID_HANDLE_VALUE)
			return;
		SetFilePointer(m_filehandle, 0, NULL, FILE_END);
		wchar_t uhead = 0xFEFF;
		DWORD dwLength;
		WriteFile(m_filehandle,&uhead,sizeof(wchar_t),&dwLength,NULL);
	}
}

bool CTwoFileStrategyLogger::Write(const CLogInfo& info)
{
	if(info.type > this->m_level)//�ڱ�ϵͳ�ڼ��д����Ϣ�ļ���
		return false;

	if(m_filehandle == INVALID_HANDLE_VALUE)
		return false;
	//����ʼ�����ļ�ʱ��ʼ��������ֹ�����߳�д��
	EnterCriticalSection(&m_cs);
	if(m_time >= CHECK_TIME)
	{
		if(!CheckFileSize(m_filehandle))
		{
			::CloseHandle(m_filehandle);
			m_current = (m_current > 0)? 0:1;
			GetFileName();			

			m_filehandle = CreateFileW(m_currentfile.c_str(),GENERIC_WRITE | GENERIC_READ, 
					FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, NULL);//��׷�ӷ�ʽ���ļ�
			if(m_filehandle == INVALID_HANDLE_VALUE)
			{
				LeaveCriticalSection(&m_cs);
				return false;
			}
			SetFilePointer(m_filehandle, 0, NULL, FILE_END);
			wchar_t uhead = 0xFEFF;
			DWORD dwLength;
			WriteFile(m_filehandle,&uhead,sizeof(wchar_t),&dwLength,NULL);
		}
		m_time = 0;
	}

	bool res = WriteData(info);
	if(res)
	{
		m_time ++;
	}
	LeaveCriticalSection(&m_cs);

	return res;

}

void CTwoFileStrategyLogger::GetFileName()
{
	wchar_t filename[MAX_PATH];
	wsprintfW(filename,L"%s%s%02d",m_directory.c_str(),m_file.c_str(),m_current);
	wstring ftmp(filename);
	m_currentfile = ftmp;
}