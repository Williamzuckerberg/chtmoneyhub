#pragma once



#define		HASH_SIZEB	16

extern bool g_isReceiveB;
/**
* ��Ӻ�����Hash
* @param 
* @param length �ļ�·�����ȡ�
* return �����Ƿ���ӳɹ���
*/
bool AddBlackHash(const unsigned char* hash);

/**
* ����Ƿ��Ǻ�������ϣֵ��
* @param hash �ļ�hashֵ��
*/
bool IsHashBlack(const unsigned char* hash);

/**
* ���hash    ==1 ���g_blackHash���ݣ� ==2 ���ȫ��
* return �����Ƿ�����ɹ���
*/
bool ClearBlackHash(ULONG index = 1);

/**
* �ж��Ƿ�ע��
*/
/*bool isRestartB();*/

/**
* �ж�ȫ�ֱ����
*/
void  getBlackTable();
/**
*����������
*/
void initialMutexB();
