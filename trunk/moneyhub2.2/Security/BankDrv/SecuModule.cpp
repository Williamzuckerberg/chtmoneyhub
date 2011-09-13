/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  SecuModule.cpp
*      ˵����  �ļ�·���������洢��
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2010.07.03	�ܼ��� ��Ҫ�Ӷ��̱߳���
*-----------------------------------------------------------*
*/

#include "ntddk.h"


#define		SECU_MODULE_BUCKET_SIZE		256

struct SECURE_PATH_ITEM
{
	struct SECURE_PATH_ITEM* next;
	UNICODE_STRING filepath;
};

// ����hash�㷨���ļ�·����ɢ�ڲ�ͬ��Ͱ�У��ӿ�����ٶȡ�
SECURE_PATH_ITEM* g_secuModules[SECU_MODULE_BUCKET_SIZE] = {0};

/**
* hash ����
* @param filepath �ļ�·����
* @param length �ļ�·�����ȡ�
* return hashֵ��
*/
ULONG Hash(const WCHAR* filepath, ULONG length)
{
	ULONG hash = 0;
	for(ULONG i = 0; i < length && filepath[i] != 0; i++)
		hash = (hash + filepath[i] * 23) % SECU_MODULE_BUCKET_SIZE;
	
	return hash;
}

bool AddSecurePath(const WCHAR* filepath, ULONG length)
{
	ULONG bucketId = Hash(filepath, length);

	KdPrint(("Add sucu module to %d: %ws\n", bucketId, filepath));

	SECURE_PATH_ITEM* pModule = (SECURE_PATH_ITEM*)ExAllocatePoolWithTag(PagedPool, sizeof(SECURE_PATH_ITEM), 'knab');
	if(pModule == NULL)
		return false;

	pModule->next = g_secuModules[bucketId];
	WCHAR* pathBuf = (WCHAR*)ExAllocatePoolWithTag(PagedPool, (length + 2) * 2, 'knab');
	if(pathBuf == NULL)
	{
		ExFreePoolWithTag(pModule, 'knab');
		return false;
	}

	memcpy((void*)pathBuf, (const void*)filepath, (length + 2) * 2);
	RtlInitUnicodeString(&pModule->filepath, pathBuf);

	g_secuModules[bucketId] = pModule;

	return true;
}

bool ClearSecurePaths()
{
	for(ULONG i = 0; i < SECU_MODULE_BUCKET_SIZE; i++)
	{
		SECURE_PATH_ITEM* curr_mod = g_secuModules[i];
		while(curr_mod)
		{
			SECURE_PATH_ITEM* next_mod = curr_mod->next;

			if(curr_mod->filepath.Buffer)
				ExFreePool(curr_mod->filepath.Buffer);
			ExFreePool(curr_mod);

			curr_mod = next_mod;
		}

		g_secuModules[i] = NULL;
	}
	return true;
}

bool IsSecurePath(PUNICODE_STRING filepath)
{
	ULONG bucketId = Hash(filepath->Buffer, filepath->Length);

	SECURE_PATH_ITEM* curr_mod = g_secuModules[bucketId];
	while(curr_mod)
	{
		// �����ļ�·���д�Сд�ַ�
		if(_wcsnicmp(filepath->Buffer, curr_mod->filepath.Buffer, curr_mod->filepath.Length) == 0)
			return true;

		curr_mod = curr_mod->next;
	}

	return false;
}