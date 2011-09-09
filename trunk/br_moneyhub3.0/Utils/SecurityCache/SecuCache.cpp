/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  SecuHash.cpp
*      ˵����  Hash�������洢��
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��

*-----------------------------------------------------------*
*/
#include "stdafx.h"
#pragma once


#include "SecuCache.h"
#include "comm.h"
#include "../../Security/Authentication/encryption/md5.h"
#include "../RecordProgram/RecordProgram.h"

CSecurityCache::CSecurityCache()
{
	m_cathfile = L"%AppData%\\MoneyHub\\SecurityCache.dat";
	m_number = 0;
	evalueCache = 0;
}

CSecurityCache::~CSecurityCache()
{
	Clear();
}
bool CSecurityCache::SetCacheFileName(wchar_t* secname)
{
	if(secname)
		m_cathfile = secname;
	return true;
}
bool CSecurityCache::Add(SecCachStruct& sec,int style)
{
	unsigned char uid = HashFunc(sec.filename);

	SecCachStruct *data = new SecCachStruct;

	memcpy(data->filename,sec.filename,sizeof(sec.filename));
	memcpy(data->chkdata,sec.chkdata,sizeof(sec.chkdata));
	if(style == 0)
		data->tag = 3;//��������
	else
		data->tag = 4;//����Ϊѧϰ������

	m_secuBucket[uid].push_back(data);
	m_isChange = true;//��������
	m_number ++;
	return true;
}
int CSecurityCache::GetFileNumber()
{
	return m_number;
}

bool CSecurityCache::Init()
{
	Clear();

	//VistaȨ������
	TCHAR szDataPath[MAX_PATH + 1];
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szDataPath);
	_tcscat_s(szDataPath, _T("\\MoneyHub"));

	WCHAR szAppDataPath[MAX_PATH + 1];
	ExpandEnvironmentStringsW(szDataPath, szAppDataPath, MAX_PATH);

	::CreateDirectoryW(szAppDataPath, NULL);

	WCHAR expName[MAX_PATH] ={0};
	ExpandEnvironmentStringsW(m_cathfile, expName, MAX_PATH);
	HANDLE hFile;
	/*if(true == isonce)
	{
		hFile = CreateFileW(expName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	}
	else
	{*/
	// ��ȡԭ����cache�ļ�
	hFile = CreateFileW(expName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//}

	if(hFile == INVALID_HANDLE_VALUE)
	{
		int error = ::GetLastError();
		CRecordProgram::GetInstance()->FeedbackError(L"SeCathe", 1000, CRecordProgram::GetInstance()->GetRecordInfo(L"��%sʧ��:%d", expName, error));
		return false;
	}

	DWORD dwLength = GetFileSize(hFile, NULL);
	if(dwLength <= 0)
	{
		CloseHandle(hFile);
		return true;
	}
	unsigned char* lpBuffer = new unsigned char[dwLength + 1];

	if (lpBuffer == NULL)
	{
		::MessageBoxW(NULL, L"�ڴ�ռ���",L"��ȫ����",MB_OK | MB_SETFOREGROUND);
		CloseHandle(hFile);
		return false;
	}

	DWORD dwRead = 0;
	if (!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
	{
		delete []lpBuffer;
		CloseHandle(hFile);
		::MessageBoxW(NULL, L"����ȫ����ʧ��",L"��ȫ����",MB_OK | MB_SETFOREGROUND);
		return false;
	}
	CloseHandle(hFile);
	
	unsigned char* unPackBuf = new unsigned char[dwRead];
	if(unPackBuf == NULL)
	{
		delete []lpBuffer;
		::MessageBoxW(NULL, L"�ڴ�ռ���",L"��ȫ����",MB_OK | MB_SETFOREGROUND);
		return false;
	}

	int ret = CacheUnPack(lpBuffer,dwRead,unPackBuf);

	delete []lpBuffer;

	if(ret < 0)
	{
		CRecordProgram::GetInstance()->FeedbackError(L"SeCathe", 1000, CRecordProgram::GetInstance()->GetRecordInfo(L"��%sUnPack�쳣", expName));

		delete[] unPackBuf;
		return false;
	}

	for(DWORD i = 0;i < (unsigned long)ret;)
	{
		SecCachStruct *data = new SecCachStruct;
		memcpy(data->filename,unPackBuf + i,sizeof(data->filename));
		i += sizeof(data->filename);
		memcpy(data->chkdata,unPackBuf + i,SECURE_SIZE);
		i += SECURE_SIZE;
		memcpy(&data->tag,unPackBuf + i,sizeof(short int));//tag�ļ��ӻ����ж���
		i += sizeof(short int);
		if(data->tag != 4)//�����Ƿ�ѧϰ����
			data->tag = 1;//�ڻ����ж�����������ӵĻ���ʼ��Ϊ0,����������Ϊ1��������Ч
		unsigned char uid = HashFunc(data->filename);
		m_secuBucket[uid].push_back(data);
		m_number ++;
	}
	delete[] unPackBuf;
	m_isChange = false;
	return true;
}

unsigned char CSecurityCache::HashFunc(wchar_t *filename)
{
	int len = wcslen(filename);
	char *p = (char*)filename;
	char uid = 0;
	for(int i = 0;i < (int)(len*sizeof(wchar_t));i ++)
	{
		uid = uid^(*(p+i));
	}
	return uid;
}

bool CSecurityCache::CalculEigenvalue(SecCachStruct& sec)
{
	const int bufsize = 8 * 1024 * 1024;
	byte* buf = new byte[bufsize];

	HANDLE hFile = CreateFileW(sec.filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		int error = ::GetLastError();
		CRecordProgram::GetInstance()->RecordWarnInfo(L"SeCathe", 1000, CRecordProgram::GetInstance()->GetRecordInfo(L"CalculEigenvalue %s�쳣", sec.filename));
		delete[] buf;
		memset(sec.chkdata,0,SECURE_SIZE);
		return false;
	}

	CMD5 md5;
	DWORD read;
	while (true)
	{
		ReadFile(hFile, buf, bufsize, &read, NULL);
		if(read == 0)
			break;

		md5.MD5Update(buf, read);
	}

	CloseHandle(hFile);
	md5.MD5Final(sec.chkdata);

	delete[] buf;
	return true;

}
bool CSecurityCache::IsMZFile(const wchar_t* pfile)
{
	//FILE *file;
	//file = _wfopen(pfile,L"rb");
	//if(file == NULL)
	//{  
	//	return false; 
	//}

	//try{
	//	USHORT ibuf;
	//	fread(&ibuf,sizeof(USHORT),1,file);//PE�ļ�ͷ��ַ
	//	fclose(file);
	//	if(ibuf == IMAGE_DOS_SIGNATURE)
	//	{
	//		return true;
	//	}
	//	else
	//	{
	//		return false;
	//	}
	//}
	//catch(...)
	//{
	//	fclose(file);
	//	return false;
	//}

	// gao
	ATLASSERT (NULL != pfile && NULL != *pfile);
	if (NULL == pfile || NULL == *pfile)
		return false;

	// ���ļ�
	// 64λ��д���������⣬ֻ����readȨ�޾ͺ��ˡ�
	HANDLE hFile = CreateFile (pfile, 
								GENERIC_READ,
								FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return false;

	USHORT iBuf = 0;
	DWORD dwRead = 0;
	
	// ��ȡ�ļ��ĵ�һ��USHORT
	BOOL bRet = ReadFile (hFile, &iBuf, sizeof(USHORT), &dwRead, NULL);
	CloseHandle (hFile);
	if (FALSE == bRet || dwRead != sizeof (USHORT))
		return false;

	if (IMAGE_DOS_SIGNATURE == iBuf)
		return true;
	else
		return false;
}
bool CSecurityCache::IsInSecurityCache(SecCachStruct& sec)
{
	//����ļ���������ļ������ڣ�ʲôҲ��������Ҫ���ϲ���Ϊ�Ѿ�������
	if(PathFileExistsW(sec.filename) == false)
		return true;

	//�����MZ�ļ�
	bool ret = IsMZFile(sec.filename);
	if(ret == false)
		return true;

	if(m_number <= 0)
		return false;

	unsigned char uid = HashFunc(sec.filename);
	for(list<SecCachStruct*>::iterator ite = m_secuBucket[uid].begin();ite != m_secuBucket[uid].end();ite ++)
	{
		if(wcscmp((*ite)->filename,sec.filename) == 0)
		{
			CalculEigenvalue(sec);//��������ֵ
			if(memcmp((*ite)->chkdata,sec.chkdata,SECURE_SIZE) != 0)//˵���ļ��и��£���Ҫ���¼��
			{
				(*ite)->tag = 2;//�и��£���Ч����
				return false;
			}
			else 
			{	
				if((*ite)->tag != 4) // ѧϰ���ݲ����и���
					(*ite)->tag = 1;//����
			}
			return true;
		}
	}
	return false;
}
//���µ���ȫ�����ļ���
bool CSecurityCache::Flush()
{
	int effectnum;
	CheckUpdate(effectnum);//�õ���Ч���ݸ���
	if(false == m_isChange)
		return true;

	WCHAR expName[MAX_PATH] ={0};
	ExpandEnvironmentStringsW(m_cathfile, expName, MAX_PATH);
	HANDLE hFile;


	if(effectnum <= 0)
	{
		hFile = CreateFileW(expName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);	
		}
		return true;
	}

	SecCachStruct tp;
	DWORD len = effectnum*(sizeof(tp.filename)+SECURE_SIZE + sizeof(short int));
	unsigned char* pBuffer = new unsigned char[len];

	if(pBuffer == NULL)
		return false;

	unsigned char* cur = pBuffer;
	for(int i = 0; i < SECURE_BUCKET_SIZE; i++)
	{
		for(list<SecCachStruct*>::iterator ite = m_secuBucket[i].begin();ite != m_secuBucket[i].end();ite ++)
		{
			if(((*ite)->tag == 1) || (*ite)->tag == 3 || (*ite)->tag == 4)//��Ч����
			{
				memcpy(cur,(void*)(*ite)->filename,sizeof((*ite)->filename));
				cur += sizeof((*ite)->filename);
				memcpy(cur,(void*)(*ite)->chkdata,SECURE_SIZE);
				cur += SECURE_SIZE;
				memcpy(cur,(void*)&((*ite)->tag),sizeof(short int));
				cur += sizeof(short int);
			}
		}
	}

	int packlen = len + 128;
	unsigned char* pPackBuf = new unsigned char[packlen];

	if(pPackBuf == NULL)
	{
		delete[] pBuffer;
		return false;
	}
	
	int ret = CachePack(pBuffer,len,pPackBuf);

	delete[] pBuffer;//���建��

	if(ret < 0)
	{
		delete[] pPackBuf;
		return false;
	}
	

	hFile = CreateFileW(expName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		::MessageBoxW(NULL, L"�򿪰�ȫ����ʧ��",L"��ȫ����",MB_OK | MB_SETFOREGROUND);
		delete[] pPackBuf;
		return false;	
	}

	DWORD dwLength;
	if(!WriteFile(hFile,pPackBuf,ret,&dwLength,NULL))
	{
		::MessageBoxW(NULL, L"д��ȫ����ʧ��",L"��ȫ����",MB_OK | MB_SETFOREGROUND);
		CloseHandle(hFile);
		delete[] pPackBuf;
		return false;
	}
	CloseHandle(hFile);
	delete[] pPackBuf;
	return true;
}

void CSecurityCache::CheckUpdate(int& effectnum)
{
	effectnum = 0;
	for(int i = 0; i < SECURE_BUCKET_SIZE; i++)
	{
		for(list<SecCachStruct*>::iterator ite = m_secuBucket[i].begin();ite != m_secuBucket[i].end();ite ++)
		{
			if(((*ite)->tag == 0)||((*ite)->tag == 2))
				m_isChange = true;
			else if(((*ite)->tag == 1) || (*ite)->tag == 3 || (*ite)->tag == 4)
				effectnum ++;
		}
	}
}
bool CSecurityCache::GetEigenvalue(ModuleList& mlist)
{
	mlist.clear();
	for(int i = 0; i < SECURE_BUCKET_SIZE; i++)
	{
		for(list<SecCachStruct*>::iterator ite = m_secuBucket[i].begin();ite != m_secuBucket[i].end();ite ++)
		{
			if(((*ite)->tag == 1) || (*ite)->tag == 3 || (*ite)->tag == 4)//��Ч����
			{
				ModuleItem item;
				memcpy(item.md,(void*)(*ite)->chkdata,SECURE_SIZE);
				mlist.push_back(item);
			}
		}
	}
	return true;
}

bool CSecurityCache::GetEigenvalue(unsigned char *data,DWORD& length)
{
	// ������е�����
	int effectnum;
	CheckUpdate(effectnum);//�õ���Ч���ݸ���
	if(false == m_isChange)
		return true;

	if(effectnum <= 0)
	{
		data = NULL;
		length = 0;
		return true;
	}

	length = effectnum * SECURE_SIZE;
	unsigned char* pBuffer = new unsigned char[length];

	if(pBuffer == NULL)
		return false;

	unsigned char* cur = pBuffer;
	for(int i = 0; i < SECURE_BUCKET_SIZE; i++)
	{
		for(list<SecCachStruct*>::iterator ite = m_secuBucket[i].begin();ite != m_secuBucket[i].end();ite ++)
		{
			if(((*ite)->tag == 1) || (*ite)->tag == 3 || (*ite)->tag == 4)//��Ч����
			{
				memcpy(cur,(void*)(*ite)->chkdata,SECURE_SIZE);
				cur += SECURE_SIZE;
			}
		}
	}

	data = pBuffer;
	return true;
}

bool CSecurityCache::Clear()
{
	for(int i = 0; i < SECURE_BUCKET_SIZE; i++)
	{
		for(list<SecCachStruct*>::iterator ite = m_secuBucket[i].begin();ite != m_secuBucket[i].end();ite ++)
		{
			delete (*ite);
		}
		m_secuBucket[i].clear();
	}

	if(NULL != evalueCache)
	{
		delete[] evalueCache;
	}
	evalueCache = NULL;

	return true;
}

bool CSecurityCache::IsShouldSend()
{
	return m_isShouldSend;
}

bool CSecurityCache::SetAllDataInvalid()
{
	for(int i = 0; i < SECURE_BUCKET_SIZE; i++)
	{
		for(list<SecCachStruct*>::iterator ite = m_secuBucket[i].begin();ite != m_secuBucket[i].end();ite ++)
		{
			(*ite)->tag = 1;//��Ч����
		}
	}
	return true;

}
bool CSecurityCache::SetSend(bool isSend)
{
	m_isShouldSend = isSend;
	return true;
}
bool CSecurityCache::IsChanged()
{
	int effectnum;
	CheckUpdate(effectnum);//�õ���Ч���ݸ���
	return m_isChange;
}
