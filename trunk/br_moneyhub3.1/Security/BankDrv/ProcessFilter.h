#pragma once

#include "windef.h"

extern KMUTEX   g_addPIDMutex;
/**
* ��ӱ������̵�ID
* @param PID ��Ҫ�����Ľ���ID��
* return �����Ƿ�ɹ���
*/
BOOL AddProtectPID(ULONG PID);

/**
* ɾ���������̵�ID
* @param PID ��Ҫ�����Ľ���ID��
* return �����Ƿ�ɹ���
*/
BOOL RemoveProtectPID(ULONG PID);

/**
* �������Ƿ񱻱���
* @param PID ����ID��
* return �����Ƿ�ɹ���
*/
BOOL IsProcessProtected(ULONG PID);

/**
* ��ȡ�������Ľ��̵�����
* return ���ر������Ľ��̵�����
*/
UINT32 GetPIDNumber();

/**
* ��ȡ�������Ľ���
* @param pPIDs ��ȡ�������Ľ��̵Ļ���������Ҫ�ڵ���ʱ����á�
* @param count ��������С��һ�����Ա���count��PID��
* return ���ر������Ľ��̵�������
*/
UINT32 GetKernelPIDs(UINT32* pPIDs, UINT32 count);
UINT32 GetPidOther(UINT32 *pPids, UINT32 count);
bool getPPidNum(UINT32 *uiPPid, UINT32 count, UINT32 iNum);
/**
* �����˳��ص���
* @param ParentId ������ID
* @param ProcessId ��ǰ����ID
* @param Create �Ƿ��Ǵ������̣�TRUE���ǣ�FALSEΪ�����˳���
*/
VOID OnProcessQuit(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

VOID initialMutexAddPID();


bool IsInstall(bool bCehck = false);