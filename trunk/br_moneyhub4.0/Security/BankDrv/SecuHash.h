#pragma once

//////////////////////////////////////////////////////////////////////////

#define		HASH_SIZE	16

extern bool g_isReceive;
/**
* ��Ӱ�ȫ�ļ�Hash
* @param 
* @param length �ļ�·�����ȡ�
* return �����Ƿ���ӳɹ���
*/
bool AddSecureHash(const unsigned char* hash);

/**
* ����ļ�hash�Ƿ�ȫ��
* @param hash �ļ�hashֵ��
* return �ļ�hash��ȫ�򷵻�true�����򷵻�false��
*/
bool IsHashSecure(const unsigned char* hash);

/**
* ���hash    ==1 ���g_secuHash���ݣ� ==2 ���ȫ��
* return �����Ƿ�����ɹ���
*/
bool ClearHash(ULONG index = 1);

/**
* �ж��Ƿ�ע��
*/
bool isRestart();

/**
* �ж�ȫ�ֱ����
*/
void  getSecuTable();
/**
*����������
*/
void initialMutex();
