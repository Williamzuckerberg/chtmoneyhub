#pragma once

#define DATAPATHLEN  ( 260*sizeof(wchar_t) )
#define DATAHASHLEN  16

extern PKEVENT pEvent;
extern PKEVENT g_pEventFilterGo;

//���ݸ�ʽΪ����������(DWORD) + �ļ�1��(MAX_PATH(260)*sizeof(WCHAR)) + md5ֵ( 16 ) + �ļ�2�� + ...
/**
*
*/
bool setSigned();

/**
*
*/
bool setData(void * pPath, unsigned int pCLen, void* pHash, unsigned int pSLen);

/**
*
*/
void getData(void * pPath, void* pHash);
/**
*
*/
bool InitializeFilterGoEvent();