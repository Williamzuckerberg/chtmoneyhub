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
	char cinfo[MAX_INFO_LENGTH];
	memset(cinfo, 0, sizeof(cinfo));
	sprintf(cinfo, "%04d-%02d-%02d %02d:%02d:%02d.%03d\t%d", info.time.wYear, info.time.wMonth, info.time.wDay, 
		info.time.wHour, info.time.wMinute, info.time.wSecond, info.time.wMilliseconds,info.pid);
	string sinfo(cinfo);

	sinfo += "\t" + info.typeinfo + "\t" + info.info+"\n";
	if(m_logfile.is_open())
	{
		m_logfile << sinfo.c_str();
		m_logfile.flush();//ȷ���ɻ��洫���ļ���
		return true;
	}

	return false;
}
/**
* ����ļ���С�ĺ�����
*/
bool CFileLogger::CheckFileSize(fstream& file)
{
	if(! file.is_open())
		return false;

	file.seekg( 0 , ios::end );//��ָ���Ƶ�ĩβ
	long size = file.tellg();//���ƫ�Ƶ�ַ�Ӷ��õ��ļ���С
	if (size >= m_filesize)
		return false;
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
	m_logfile.open(m_currentfile.c_str(),ios::out | ios::app | ios::in);//��׷�ӷ�ʽ���ļ�

	if(!m_logfile.is_open())
	{
		//��һ�δ�ʧ�ܣ���ô����ʱ�ļ�
		m_currentfile = m_directory + L"~logtemp.dat";
		m_logfile.open(m_currentfile.c_str(),ios::out | ios::app | ios::in);//��׷�ӷ�ʽ���ļ�
		if(!m_logfile.is_open())
			return;
	}

	if(!CheckFileSize(m_logfile))
	{
		m_logfile.close();
		m_logfile.open(m_currentfile.c_str(),ios::trunc |ios::out | ios::in | ios::ate);
		if(!m_logfile.is_open())
			return;
	}
}

COneFileStrategyLogger::~COneFileStrategyLogger(void)
{
	m_logfile.close();
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
		if(!CheckFileSize(m_logfile))
		{
			m_logfile.close();
			m_logfile.open(m_currentfile.c_str(),ios::trunc |ios::out | ios::in | ios::ate);
			if(!m_logfile.is_open())
			{
				LeaveCriticalSection(&m_cs);
				return false;
			}
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
	m_logfile.close();
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
	m_logfile.open(m_currentfile.c_str(),ios::out | ios::app | ios::in);//��׷�ӷ�ʽ���ļ�

	if(!m_logfile.is_open())
	{
		//�򿪵�һ���ļ�ʧ�ܣ���ô����ʱ�ļ�
		m_file = L"~logtemp.d";
		GetFileName();
		m_logfile.open(m_currentfile.c_str(),ios::out | ios::app | ios::in);//��׷�ӷ�ʽ���ļ�
		//����ʱ�ļ�Ҳʧ�ܵĻ����˳�
		if(!m_logfile.is_open())
			return;
	}
	//�������־�ļ��ɹ���������־�ļ���С��������ϴ�С����ô���ø��ļ���������־�ļ�
	if(!CheckFileSize(m_logfile))
	{
		m_logfile.close();
		m_current = (m_current > 0)? 0:1;

		GetFileName();
		m_logfile.open(m_currentfile.c_str(),ios::out | ios::app | ios::in);
		if(!CheckFileSize(m_logfile))
		{	//��һ����־�ļ�Ҳ���ˣ���ô�õ�һ����־�ļ�
			m_logfile.close();
			m_current = (m_current > 0)? 0:1;
			GetFileName();
			m_logfile.open(m_currentfile.c_str(), ios::trunc |ios::out | ios::in | ios::ate);// ios::app |
		}
		if(!m_logfile.is_open())
			return;
	}
}

bool CTwoFileStrategyLogger::Write(const CLogInfo& info)
{
	if(info.type > this->m_level)//�ڱ�ϵͳ�ڼ��д����Ϣ�ļ���
		return false;

	if(!m_logfile.is_open())
		return false;
	//����ʼ�����ļ�ʱ��ʼ��������ֹ�����߳�д��
	EnterCriticalSection(&m_cs);
	if(m_time >= CHECK_TIME)
	{
		if(!CheckFileSize(m_logfile))
		{
			m_logfile.close();
			m_current = (m_current > 0)? 0:1;
			GetFileName();			

			m_logfile.open(m_currentfile.c_str(),ios::trunc | ios::out | ios::ate | ios::in);
			if(!m_logfile.is_open())
			{
				LeaveCriticalSection(&m_cs);
				return false;
			}
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