/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  FileLogger.h
 *      ˵����  �ļ���־�����������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */


#pragma once
#include "ILogger.h"
#include <string>
#include "windows.h"
using namespace std;

#define CHECK_TIME			20	//��д���ļ�����һ�����������ļ���С

/**
* �ļ���־д��Ľӿڼ�������
* �̳���Ilogger��
*/
class CFileLogger:public ILogger
{
public:
	CFileLogger(void);
	~CFileLogger(void);
protected:
	long m_filesize;		//���ü�¼�ļ���С
	//Ϊ�˷�ֹ����·�������ڳ������漰���ļ�·�����ƶ�ʹ�ÿ��ֽ�
	wstring m_file;			//��¼Ӧ�ô洢���ļ�����
	wstring m_directory;	//�洢��־�ļ���ȫ·��
	wstring m_currentfile;	//��¼ʵ�ʴ洢ȫ·���ļ���
	HANDLE	m_filehandle;		//ʵ�ʴ򿪵��ļ�
	CRITICAL_SECTION m_cs;	//����ͬ������ͬʱ���øú����������Դ����
	
public:
	/**
	* ������־����ļ���С��
	* @param size��long �ļ���С��
	*/
	void SetFileSize(long size);

	/**
	* ������־�ļ����ơ�
	* @param filename��wstring �ļ����ơ�
	*/
	void SetFileName(const wstring& filename);
	

protected:

	/**
	* ���ļ���д����־��Ϣ�ĺ�����
	* @param info��CLogInfo ��־��Ϣ��
	* @return bool д���Ƿ�ɹ�
	* ֱ�����Ѿ��򿪵�m_logfile�ļ���д����Ϣ
	*/
	bool WriteData(const CLogInfo& info);

	/**
	* ����ļ���С�ĺ�����
	* @param file��fstream �ļ�����
	* @return bool �ж��Ƿ���Ϲ涨��С�����Ϸ���true�������Ϸ���false
	* ����ļ���С�Ƿ���Ϲ涨�ĺ�������ʹ�øú���ʱ��ȷ��file�ļ����Ѿ���
	*/
	bool CheckFileSize(HANDLE& hFile);
};
/**
* ���ļ���־����д����
* �̳���CFileLogger��
*/
class COneFileStrategyLogger:public CFileLogger
{
public:
	~COneFileStrategyLogger(void);
	/**
	* ���캯����
	* @param logdirectory��wstring ��־�ļ��С�
	* @param logfilename��wstring ��־�ļ���
	* @param size��long ��־����ļ���С��
	* ��ʼ��������
	*/
	COneFileStrategyLogger(wstring logdirectory = L".\\Logs",wstring logfilename = L"log",long size = 10000000);
public:
	/**
	* д����־��Ϣ������
	* @param CLogInfo& ��Ϣ��ṹ��
	* ����ϵͳ��д����Ϣʱ�����øú���д��,������ILogger��ĸú�����
	*/
	bool Write(const CLogInfo& info);
private:
	int m_time;		//��¼����ļ�д�����������������һ������ʱ�Ž��м���С�Ĳ�����Ȼ����ոò���
};
/**
* ˫�ļ���־����д����
* �̳���CFileLogger��
*/
class CTwoFileStrategyLogger:public CFileLogger
{
public:
	~CTwoFileStrategyLogger(void);
	/**
	* ���캯����
	* @param logdirectory��wstring ��־�ļ��С�
	* @param logfilename��wstring ��־�ļ���
	* @param size��long ��־����ļ���С��
	* ��ʼ��������
	*/
	CTwoFileStrategyLogger(wstring logdirectory = L".\\Logs",wstring logfilename = L"log",long size = 10000000);

public:
	/**
	* д����־��Ϣ������
	* @param CLogInfo& ��Ϣ��ṹ��
	* ����ϵͳ��д����Ϣʱ�����øú���д��,������ILogger��ĸú�����
	*/
	bool Write(const CLogInfo& info);
private:
	int m_time;		//��¼����ļ�д�����������������һ������ʱ�Ž��м���С�Ĳ�����Ȼ����ոò���
	int m_current;	//��¼��ǰʹ�õ��ļ��������ֲ���
private:
	void GetFileName();
};

//��3��������δ���
/*class CMutiLevelFileStrategyLogger:public CFileLogger
{
public:
	~CMutiLevelFileStrategyLogger(void);
	CMutiLevelFileStrategyLogger(wstring logfilename = L"log",long size = 10000000);

public:
	bool Write(const CLogInfo& info);
};*/
