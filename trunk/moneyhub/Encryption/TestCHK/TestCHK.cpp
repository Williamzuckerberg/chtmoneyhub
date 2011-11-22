// TestCHK.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <iostream>
#include "../SHA1/sha.h"
#include "../CHKFile/CHK.h"

int CheckMoneyHubList(unsigned char *buffer, int length, const char *path, char *message)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	char fileName[1024];
	//	int nameLength;
	unsigned char md[20];
	SHA_CTX	c;
	//	unsigned char *content;
	//	int contentLength;
	int bufferLength;
	unsigned char *ptr;
	int pos;

	//assert(buffer && path && message);

	ptr=buffer;
	bufferLength=ptr[0]|(ptr[1]<<8)|(ptr[2]<<16)|(ptr[3]<<24);

	if (bufferLength+4!=length)
	{
		strcpy(message,"������Ϣ��ʽ����");
		return -3010;
	}
	ptr+=4;
	pos=4;
	while (pos<length)
	{
		std::string strFile = path;

		bufferLength=ptr[0]|(ptr[1]<<8);
		if (pos+2+bufferLength>length)
		{
			strcpy(message,"������Ϣ��ʽ����");
			return -3011;
		}
		ptr=ptr+2;
		pos=pos+2;

		memcpy(fileName,ptr,bufferLength);
		fileName[bufferLength]=0;
		ptr+=bufferLength;
		pos+=bufferLength;

		strFile =strFile + fileName;

		hFile = CreateFile(strFile.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			// ��ȡ�ļ�����
			DWORD dwLength = GetFileSize(hFile, NULL);
			char* lpBuffer = new char[dwLength + 1];

			if(lpBuffer==NULL)
			{
				strcpy(message,"�ڴ�ռ���");
				return -3001;
			}

			DWORD dwRead = 0;
			if(!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
			{
				delete []lpBuffer;
				sprintf(message,"���ļ�[%s]ʧ��",strFile.c_str());
				return -3002;
			}

			CloseHandle(hFile);

			SHA1_Init(&c);
			SHA1_Update(&c,(const void *)lpBuffer,dwRead);
			SHA1_Final(md, &c);

			delete []lpBuffer;

			if (memcmp(md,ptr,20)!=0)
			{
				sprintf(message,"�ļ�[%s]���۸�",strFile.c_str());
				return -3012;
			}

			ptr+=20;
			pos+=20;
		} else {
			sprintf(message,"�Ҳ����ļ�[%s]",strFile.c_str());
			return -3013;
		}
	}
	if (pos<length)
	{
		strcpy(message,"������Ϣ��ʽ����");
		return -3014;
	}

	strcpy(message,"�ɹ�");
	return 0;
}

int VerifyMoneyHubList(const char *path, const char* CHKFileName, char *message)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	unsigned char *content;
	int contentLength;
	int ret;

	//assert(path && message);

	std::string strFile = CHKFileName;

	// ���ļ�
	hFile = CreateFile(strFile.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// ��ȡ�ļ�����
		DWORD dwLength = GetFileSize(hFile, NULL);
		unsigned char* lpBuffer = new unsigned char[dwLength + 1];

		if(lpBuffer==NULL)
		{
			strcpy(message,"�ڴ�ռ���");
			return -3001;
		}

		DWORD dwRead = 0;
		if(!ReadFile(hFile, lpBuffer, dwLength, &dwRead, NULL))
		{
			delete []lpBuffer;
			strcpy(message,"���ƽ���������������Ϣʧ��");
			return -3002;
		}
		CloseHandle(hFile);

		content=new unsigned char[dwRead];

		if (content==NULL)
		{
			delete []lpBuffer;
			strcpy(message,"�ڴ�ռ���");
			return -3001;
		}

		contentLength=unPackCHK(lpBuffer,dwRead,content);

		delete []lpBuffer;

		if (contentLength<0)
		{
			strcpy(message,"����ƽ���������������Ϣʧ��");
			return -3003;
		}

		ret=CheckMoneyHubList(content,contentLength,path,message);

		delete []content;

		return ret;


	} else {
		strcpy(message,"�Ҳ����ƽ���������������Ϣ");
		return -3000;
	}

}

int _tmain(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Please input root dir!" << std::endl;
		return 0;
	}

	std::cout << "start test CHK file..." << std::endl;
	std::string strFileList = argv[1];
	char message[100];

	std::string strCHK = strFileList;
	strCHK += "\\MoneyHub.mchk";

	int ret = VerifyMoneyHubList(strFileList.c_str(), strCHK.c_str(), message);

	printf("return [%d], message[%s]\n",ret,message);
	return 0;
}

