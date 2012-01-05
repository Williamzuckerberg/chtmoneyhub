/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  ServiceTable.cpp
*      ˵����  SSDT������
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2010.07.03	�ܼ���
*-----------------------------------------------------------*
*/

#include "ntddk.h"
#include "ServiceTable.h"
#include "MemoryProtect.h"

typedef struct _SYSTEMSERVICEDESCRIPTORTABLE
{
	PVOID*   ServiceTableBase;
	PULONG   ServiceCounterTableBase;
	ULONG    NumberOfService;
	ULONG    ParamTableBase;
}SYSTEMSERVICEDESCRIPTORTABLE,*PSYSTEMSERVICEDESCRIPTORTABLE;

extern "C" PSYSTEMSERVICEDESCRIPTORTABLE KeServiceDescriptorTable;


/**
*   ���ù��ӷ�����
*/
PVOID UpdateService(ULONG ServiceID, PVOID ServiceProc)
{
	if(ServiceID >= KeServiceDescriptorTable->NumberOfService)
		return NULL;

	ULONG OldAttr;

	DisableWriteProtect(&OldAttr);
	PVOID PreviousProc = KeServiceDescriptorTable->ServiceTableBase[ServiceID];
	KeServiceDescriptorTable->ServiceTableBase[ServiceID] = ServiceProc;
	EnableWriteProtect(OldAttr);
	
	KdPrint(("Service Update: 0x%08x -> 0x%08x\n", PreviousProc, ServiceProc));

	return PreviousProc;
}

/**
*  ��鹳��״̬����
*/
bool _checkHookSafeMapV(PVOID pFun,ULONG id)
{
	if(id >= KeServiceDescriptorTable->NumberOfService)
		return false;

	ULONG OldAttr;
	bool bReturn=false;

	//DisableWriteProtect(&OldAttr);
	PVOID PreviousProc = KeServiceDescriptorTable->ServiceTableBase[id];
	bReturn = PreviousProc==pFun? true:false;
	//EnableWriteProtect(OldAttr);

	return bReturn;
}