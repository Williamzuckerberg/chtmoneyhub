#pragma once

/**
* ����ϵͳ����
* @param ServiceID ����ID��
* @param ServiceProc ϵͳ�������̡�
* return ����ԭϵͳ��������
*/
PVOID UpdateService(ULONG ServiceID, PVOID ServiceProc);

/**
* ��������
* @param id ����ID��
* @param pFun �������̡�
* return ���ط����Ƿ���������
*/
bool _checkHookSafeMapV(PVOID pFun,ULONG id);