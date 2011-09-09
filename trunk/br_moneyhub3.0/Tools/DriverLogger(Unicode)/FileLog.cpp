/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  FileLog.cpp
 *      ˵����  ������־�������ӿڿ�ʵ���ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.08.06	������	��ʼ�汾
 *  1.0.1   2010.08.09  ������  �޸�ΪUnicode�汾
 *
 *  ����������
 *  Visual Studio 2008+WinDDK\7600.16385.1
 *	 Include��:WinDDK\7600.16385.1\inc\ddk;D:\WinDDK\7600.16385.1\inc\crt;WinDDK\7600.16385.1\inc\api
 *	 Lib��:WinDDK\7600.16385.1\lib\wxp\i386\
 *			hal.lib int64.lib ntoskrnl.lib ntstrsafe.lib
 *
 *-----------------------------------------------------------*
 */

#include "FileLog.h"
#include "ntddk.h"
//#define LOGFILENAME_TAG "logf"	//��־�ļ���tag
#define MAX_PATH 260						//��־�ļ������ַ�������󳤶�
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include "LogConst.h"
#define CHECK_TIME 20						//����ô������ļ����д�С�ж�

UNICODE_STRING  g_logFileName = {0};		//��¼��־�ļ�����
ULONG			g_logLevel = LOG_TYPE_ALL;	//��¼��־�ļ������Ϣ�ȼ�
HANDLE			g_logFile = NULL;			//��־�ļ����
LONGLONG		g_logFileMaxSize = 0;		//��¼��־�ļ���������С
ULONG			g_logCurrentFileNumber = 0;	//��0-1�����ļ��洢��־��Ҫ�뵱ǰ���ļ���ͬ������
USHORT			g_logWriteTime = 0;			//��¼д�����������CHECK_TIME������
USHORT			g_logStrategy = 1;			//��¼��־�ļ�����,1Ϊ���ļ���2Ϊ˫�ļ�

KMUTEX			g_logMutex;					//�����壬��ֹ���̵߳���д��־

/**
* ����ϵͳ����־��¼�ȼ�
* @param loglevel:ULONG,��־�ܹ���¼��Ϣ�����ȼ���
*/
void SetLogLevel(ULONG loglevel)
{
	g_logLevel = loglevel;
}

/**
* ���ϵͳ����־��¼�ȼ�
* @return ULONG ���������ܹ���¼��Ϣ�����ȼ���
*/
ULONG GetLogLevel()
{
	return g_logLevel;
}

/**
* �����־����
* �޲���
*/
void LogUninitialize()
{
	//�ر��ļ�

	g_logFile = 0;
	g_logLevel = LOG_TYPE_ALL;

	::RtlFreeUnicodeString(&g_logFileName);
	g_logCurrentFileNumber = 0;
	g_logFileMaxSize = 0;
	g_logWriteTime = 0;
	g_logStrategy = 1;
	KdPrint( ("Log uninitialize success!\n") );

}

/**
* ��ȡ��ǰϵͳʱ��
* @return TIME_FIELDS ���ص�ǰϵͳ��ʱ�䡣
*/
TIME_FIELDS GetLocalTime()
{
	LARGE_INTEGER  current_stime;
	KeQuerySystemTime(&current_stime);

	LARGE_INTEGER current_ltime;
	ExSystemTimeToLocalTime(&current_stime,&current_ltime);

	TIME_FIELDS current_tinfo;
	RtlTimeToTimeFields(&current_ltime,&current_tinfo);
    
	return current_tinfo;
}

/**
* ����ļ���С������������ļ������涨��С���´����ļ���2������
* ������Ҫ�����ؽ������ѡ������־ϵͳ��ʼ��ʱһ�����1��2����д����־�ļ����жϴ�С����1.
*/
NTSTATUS CheckLogFileSizeAndReCreateLogFile(PUNICODE_STRING pRecreateFilename,USHORT strategy,HANDLE& logFile)
{
	OBJECT_ATTRIBUTES    objectAttributes;
	IO_STATUS_BLOCK		ioStatus;
	//��ȡ�ļ���С
	FILE_STANDARD_INFORMATION  fsi;

	NTSTATUS ntStatus = ::ZwQueryInformationFile(logFile,&ioStatus,&fsi,sizeof(FILE_STANDARD_INFORMATION),FileStandardInformation);

	
	if(!NT_SUCCESS(ntStatus) )
	{
		KdPrint( ("Strategy:%d:Get file's length error!\n",g_logStrategy) );
		return ntStatus;
	}

	if(fsi.EndOfFile.QuadPart >= g_logFileMaxSize)//�ļ�������С,�رո��ļ�,�ؽ�
	{
		ZwClose(logFile);
		InitializeObjectAttributes(&objectAttributes,pRecreateFilename,OBJ_CASE_INSENSITIVE,NULL, NULL);

		if(strategy == 1)
		{
			KdPrint( ("Recreate File\n") );
			ntStatus = ZwCreateFile(&logFile,FILE_READ_ATTRIBUTES | FILE_APPEND_DATA | SYNCHRONIZE,
						&objectAttributes,&ioStatus,NULL,FILE_ATTRIBUTE_NORMAL,
						FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_SUPERSEDE,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);

			if(!NT_SUCCESS(ntStatus))
			{
				//���ڲ���1��Ӱ�죬���Ϊ����2��g_logCurrentFileNumber���ظ�Ϊԭ��ֵ
				g_logCurrentFileNumber = (g_logCurrentFileNumber > 0) ? 0 : 1;
				KdPrint( ("Strategy:%d:ReCreate File Error!\n",g_logStrategy));
				return ntStatus;
			}
			WCHAR uhead = 0xFEFF;
			ntStatus = ZwWriteFile(logFile,NULL,NULL,NULL,&ioStatus,&uhead,sizeof(WCHAR),NULL,NULL);//д�����Unicode�ļ���ʶ�ŵ���ͷ
			//ZwClose(logFile);
		}
		else
		{
			KdPrint( ("Open Another File\n") );
			ntStatus = ZwCreateFile(&g_logFile,FILE_READ_ATTRIBUTES | FILE_APPEND_DATA | SYNCHRONIZE,
								&objectAttributes,&ioStatus,NULL,FILE_ATTRIBUTE_NORMAL,
								FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_OPEN_IF,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);

			if(!NT_SUCCESS(ntStatus))
			{
				//���ڲ���1��Ӱ�죬���Ϊ����2��g_logCurrentFileNumber���ظ�Ϊԭ��ֵ
				g_logCurrentFileNumber = (g_logCurrentFileNumber > 0) ? 0 : 1;
				KdPrint( ("Strategy:%d:ReCreate File Error!\n",g_logStrategy));
				return ntStatus;
			}

		}
	}
	else
		g_logCurrentFileNumber = (g_logCurrentFileNumber > 0) ? 0 : 1;//���ڲ���1��Ӱ�죬���Ϊ����2��g_logCurrentFileNumber���ظ�Ϊԭ��ֵ

	return STATUS_SUCCESS;
}

/**
* ��ʼ���ļ���־��4������
* ��ʹ�ø���־ģ��ʱ��д��־֮ǰҪ���øú������г�ʼ�������м��κεط������Զ���־�ļ�����д����־��Ϣ����д����־�󣬲��ٵ��øú���д��־����ôҪ����LogUninitialize��������
*/
void LogInitialize(ULONG loglevel,WCHAR* logfilename,LONGLONG filesize,USHORT strategy)
{
	size_t flength;
	NTSTATUS ntStatus = RtlStringCchLengthW(logfilename,(MAX_PATH-2),&flength);
	if(!NT_SUCCESS(ntStatus) )
	{
		KdPrint( ("Filename too long!\n") );
		return;
	}

	KdPrint( ("Filename length:%d\n",flength) );

	//���ļ�����¼��g_LogFileName��,ע��Unicode_String������0Ϊ��������Length��Ϊ�����׼
	g_logFileName.Buffer = (PWSTR)ExAllocatePool(NonPagedPool,MAX_PATH*sizeof(WCHAR));//����˫���ڴ�
	RtlZeroMemory(g_logFileName.Buffer,MAX_PATH*sizeof(WCHAR));//��0��Ϊ������Ĳ���˳��
	g_logFileName.MaximumLength = MAX_PATH*sizeof(WCHAR);
	RtlCopyMemory(g_logFileName.Buffer,logfilename,flength*sizeof(WCHAR));
	g_logFileName.Length = flength*sizeof(WCHAR);

	KdPrint( ("Filename:%wZ\n",&g_logFileName) );

	g_logLevel = loglevel;
	g_logFileMaxSize = filesize;
	g_logWriteTime = 0;

	g_logStrategy = strategy;

	//��ʼ����������
	KeInitializeMutex(&g_logMutex,0);
	KdPrint( ("Log Init Sucsess!\n") );
}

/**
* ��ȡ��ǰ�ļ�����
* @param pfactfilename:PUNICODE_STRING,��Ҫ��ǰ���壬�ڱ������ڽ����˿ռ����룬�������ɵ������ͷţ�����
* @return USHORT ����ת���Ľ��:1��ʾ�ɹ���0��ʾʧ�ܡ�
* ����g_CurrentFileNumber��g_logFile��ͬ�������µ��ļ�����
*/
USHORT GetLogFileName(OUT PUNICODE_STRING pfactfilename)
{
	//�˴���factfilename���������룬�ڱ�������δ�������٣������ⲿҪ����
	pfactfilename->Buffer = (PWSTR)ExAllocatePool(PagedPool,MAX_PATH*sizeof(WCHAR));
	RtlZeroMemory(pfactfilename->Buffer,MAX_PATH*sizeof(WCHAR));
	pfactfilename->MaximumLength = MAX_PATH*sizeof(WCHAR);

	NTSTATUS ntStatus = RtlStringCchPrintfW(pfactfilename->Buffer,MAX_PATH*sizeof(WCHAR),L"%s%02d",g_logFileName.Buffer,g_logCurrentFileNumber);
	
	if(!NT_SUCCESS(ntStatus) )
	{
		KdPrint( ("Filename error!\n") );
		return 0;
	}
	pfactfilename->Length = g_logFileName.Length + 2*sizeof(WCHAR);
	return 1;
}

/**
* д��־����
* @param iLevel:ULONG,Ҫд����־��Ϣ�ĵȼ�����ο�LogConst.h�ļ��ڵȼ���
* @param format:NTSTRSAFE_PSTR*,������Ϣ�ĸ�ʽ���˸�ʽ�ο�printf��
* @param ...:���,�ο�printf��Ҫд�����Ϣ��
* д��־��Ϣ�ӿڣ�д��־��Ϣ��С�벻Ҫ�����涨���ȵĴ�С�������ʱΪ256�ֽڣ���
*/
void _cdecl WriteSysLog(ULONG iLevel, NTSTRSAFE_PWSTR format,...)
{
	//�жϼ�¼��־����
	if(iLevel > g_logLevel)
		return;

	//��û����壬������ĵ����У����еĺ����˳����ֶ�Ҫ�����ͷŻ�����
	KeWaitForSingleObject(&g_logMutex, Executive, KernelMode, FALSE, NULL);
	
	HANDLE logfile;
	OBJECT_ATTRIBUTES   objectAttributes;
	IO_STATUS_BLOCK		ioStatus;
	NTSTATUS ntStatus;
	
	InitializeObjectAttributes(&objectAttributes,&g_logFileName,OBJ_CASE_INSENSITIVE,NULL, NULL);

	
	ntStatus = ZwCreateFile(&logfile,FILE_READ_ATTRIBUTES | FILE_APPEND_DATA | SYNCHRONIZE,
				&objectAttributes,&ioStatus,NULL,FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_OPEN_IF,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
	if(!NT_SUCCESS(ntStatus) )
	{
		KeReleaseMutex(&g_logMutex,FALSE);
		KdPrint( ("Strategy:%d:Init Open file error!\n",g_logStrategy) );
		return;
	}

	//�ж��ļ���С����ĺ���
	if(g_logWriteTime >= CHECK_TIME)
	{
		if(g_logStrategy == 1)
		{
			ntStatus = CheckLogFileSizeAndReCreateLogFile(&g_logFileName,1,logfile);

			if(!NT_SUCCESS(ntStatus) )
			{
				KeReleaseMutex(&g_logMutex,FALSE);
				KdPrint( ("Strategy:%d:Init Open file error!\n",g_logStrategy) );
				return;
			}
		}

		g_logWriteTime = 0;
	}

	TIME_FIELDS ctime;
	ctime = GetLocalTime();
	ULONG pid = (ULONG)PsGetCurrentProcessId();
	PWCHAR ptype;

	switch(iLevel)
	{
		case LOG_TYPE_ERROR:	ptype = L"ERROR";break;
		case LOG_TYPE_WARN:		ptype = L"WARNING";break;
		case LOG_TYPE_INFO:		ptype = L"INFO";break;
		default:				ptype = L"DEBUG";break;
	}
    WCHAR strTemp[MAX_INFO_LENGTH];
    RtlZeroMemory(strTemp, MAX_INFO_LENGTH);
    NTSTRSAFE_PWSTR pinfo = strTemp;

 	va_list args; 
    va_start(args,format); 
	ntStatus =::RtlStringCchVPrintfW(pinfo,MAX_INFO_LENGTH*sizeof(WCHAR),format,args);
	if( !NT_SUCCESS(ntStatus) )
	{
		DbgPrint( ("Log info conversion error1!\n") );
		ZwClose(logfile);
		KeReleaseMutex(&g_logMutex,FALSE);
		//�ں������еĳ����ͷŻ�����
		return;
	}
    va_end(args); 

	WCHAR cinfo[MAX_INFO_LENGTH*2];
	RtlZeroMemory(cinfo,MAX_INFO_LENGTH*2*sizeof(WCHAR));//��0��Ϊ������Ĳ���˳��
	ntStatus = RtlStringCchPrintfW(cinfo,MAX_INFO_LENGTH*2*sizeof(WCHAR), L"%04d-%02d-%02d %02d:%02d:%02d.%03d\t%d\t%s\t%s\r\n", ctime.Year, ctime.Month, ctime.Day, 
		ctime.Hour, ctime.Minute, ctime.Second, ctime.Milliseconds,pid,ptype,pinfo);
	if( !NT_SUCCESS(ntStatus) )
	{
		DbgPrint( ("Log info conversion error2!\n") );
		//�ں������еĳ����ͷŻ�����
		ZwClose(logfile);
		KeReleaseMutex(&g_logMutex,FALSE);
		return;
	}

	size_t length;
	ntStatus = RtlStringCchLengthW(cinfo,MAX_INFO_LENGTH*2,&length);
	if( !NT_SUCCESS(ntStatus) )
	{
		DbgPrint( ("Log info conversion error3!\n") );
		//�ں������еĳ����ͷŻ�����
		ZwClose(logfile);
		KeReleaseMutex(&g_logMutex,FALSE);
		return;
	}
	ntStatus = ZwWriteFile(logfile,NULL,NULL,NULL,&ioStatus,cinfo,length*sizeof(WCHAR),NULL,NULL);
	if( !NT_SUCCESS(ntStatus) )
	{
		DbgPrint( ("Write log error!\n") );
		ZwClose(logfile);
		//�ں������еĳ����ͷŻ�����
		KeReleaseMutex(&g_logMutex,FALSE);
		return;
	}
	g_logWriteTime ++;//����д�����

	ZwClose(logfile);
	KeReleaseMutex(&g_logMutex,FALSE);
	//KdPrint( ("write log successful!\n") );
}


/**
*        �����־����
*/
bool   deleteLogFileA()
{
	HANDLE logFile = NULL;
	OBJECT_ATTRIBUTES    objectAttributes;
	IO_STATUS_BLOCK		ioStatus;

	InitializeObjectAttributes(&objectAttributes,&g_logFileName,OBJ_CASE_INSENSITIVE,NULL, NULL);
	ZwCreateFile(&logFile,FILE_READ_ATTRIBUTES | FILE_APPEND_DATA | SYNCHRONIZE,
		&objectAttributes,&ioStatus,NULL,FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_SUPERSEDE,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);

	ZwClose(logFile);

	return true;
}