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
#include <iostream>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include <io.h>
#include <FCNTL.H>

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
	swprintf(cinfo, L"%04d-%02d-%02d %02d:%02d:%02d.%03d\t%d", info.time.wYear, info.time.wMonth, info.time.wDay, 
		info.time.wHour, info.time.wMinute, info.time.wSecond, info.time.wMilliseconds,info.pid);
	wstring sinfo(cinfo);

	sinfo += L"\t" + info.typeinfo + L"\t" + info.info + L"\r\n";


	char *pdata = (char*)sinfo.c_str();
	if(m_plogfile)
	{
		int wl = fwrite(pdata,(sinfo.length())*sizeof(wchar_t),1,m_plogfile);
		
		if(wl <= 0)
			return false;
		fflush(m_plogfile);
		return true;
	}

	return false;
}
/**
* ����ļ���С�ĺ�����
*/
bool CFileLogger::CheckFileSize(FILE *pFile)
{
	if(!pFile)
		return false;

	long fsize;
	int re = fseek(pFile,0,SEEK_END);

	if(re != 0)
		return false;

	fsize = ftell(pFile);
	if(fsize == 0)
	{
		wchar_t uhead = 0xFEFF;
		fwrite(&uhead,1,sizeof(wchar_t),pFile);
	}
	if(fsize >= m_filesize)
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
	//Ϊ�˷�ֹ��������,��һ������ͳһʹ�ÿ��ֽ�
	m_plogfile = _wfopen(m_currentfile.c_str(),L"ab");//��׷�ӷ�ʽ���ļ�

	if(!m_plogfile)
	{
		//��һ�δ�ʧ�ܣ���ô����ʱ�ļ�
		m_currentfile = m_directory + L"~logtemp.dat";
		m_plogfile = _wfopen(m_currentfile.c_str(),L"ab");//��׷�ӷ�ʽ���ļ�
		if(!m_plogfile)
			return;
	}

	if(!CheckFileSize(m_plogfile))
	{
		fclose(m_plogfile);
		m_plogfile = _wfopen(m_currentfile.c_str(),L"wb");
		
		if(!m_plogfile)
			return;
		wchar_t uhead = 0xFEFF;
		fwrite(&uhead,1,sizeof(wchar_t),m_plogfile);
	}
}

COneFileStrategyLogger::~COneFileStrategyLogger(void)
{
	fclose(m_plogfile);
}

bool COneFileStrategyLogger::Write(const CLogInfo& info)
{
	//ÿд��һ��������
	if(info.type > this->m_level)//�ڱ�ϵͳ�ڼ��д����Ϣ�ļ���
		return false;
	//����ʼ�����ļ�ʱ��ʼ��������ֹ�����߳�д��
	EnterCriticalSection(&m_cs);
	if(m_time >= CHECK_TIME)
	{
		if(!CheckFileSize(m_plogfile))
		{
			fclose(m_plogfile);
			m_plogfile = _wfopen(m_currentfile.c_str(),L"wb");
			if(!m_plogfile)
			{
				LeaveCriticalSection(&m_cs);
				return false;
			}
			wchar_t uhead = 0xFEFF;
			fwrite(&uhead,1,sizeof(wchar_t),m_plogfile);
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
	fclose(m_plogfile);
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
	m_plogfile = _wfopen(m_currentfile.c_str(),L"ab");//��׷�ӷ�ʽ���ļ�
	if(!m_plogfile)
	{
		//�򿪵�һ���ļ�ʧ�ܣ���ô����ʱ�ļ�
		m_file = L"~logtemp.d";
		GetFileName();
		m_plogfile = _wfopen(m_currentfile.c_str(),L"ab");//��׷�ӷ�ʽ���ļ�
		//����ʱ�ļ�Ҳʧ�ܵĻ����˳�
		if(!m_plogfile)
			return;
	}
	//�������־�ļ��ɹ���������־�ļ���С��������ϴ�С����ô���ø��ļ���������־�ļ�
	if(!CheckFileSize(m_plogfile))
	{
		fclose(m_plogfile);
		m_current = (m_current > 0)? 0:1;

		GetFileName();
		m_plogfile = _wfopen(m_currentfile.c_str(),L"ab");
		if(!m_plogfile)
			return;
		wchar_t uhead = 0xFEFF;
		fwrite(&uhead,1,sizeof(wchar_t),m_plogfile);
	}
}

bool CTwoFileStrategyLogger::Write(const CLogInfo& info)
{
	if(info.type > this->m_level)//�ڱ�ϵͳ�ڼ��д����Ϣ�ļ���
		return false;

	if(!m_plogfile)
		return false;
	//����ʼ�����ļ�ʱ��ʼ��������ֹ�����߳�д��
	EnterCriticalSection(&m_cs);
	if(m_time >= CHECK_TIME)
	{
		if(!CheckFileSize(m_plogfile))
		{
			fclose(m_plogfile);
			m_current = (m_current > 0)? 0:1;
			GetFileName();			

			m_plogfile = _wfopen(m_currentfile.c_str(),L"wb");
			if(!m_plogfile)
			{
				LeaveCriticalSection(&m_cs);
				return false;
			}
			wchar_t uhead = 0xFEFF;
			fwrite(&uhead,1,sizeof(wchar_t),m_plogfile);
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