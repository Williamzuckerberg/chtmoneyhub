/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  QueryObjectName.cpp
*      ˵����  ��ȡ�ļ�·����
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2010.07.03	�ܼ���
*-----------------------------------------------------------*
*/


#include "ntddk.h"
#include "undoc.h"

POBJECT_NAME_INFORMATION BkQueryNameString(IN PVOID Object)
{
	ULONG ReturnLength = 1024;

	POBJECT_NAME_INFORMATION ObjectNameInfo = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag( PagedPool, ReturnLength, 'knab' );

	if ( ObjectNameInfo != NULL ) 
	{
		RtlZeroMemory(ObjectNameInfo,ReturnLength);//��ʼ���ڴ�Ϊ0����ֹ����,fanzhenxing ����
		NTSTATUS Status = ObQueryNameString(Object, ObjectNameInfo, ReturnLength, &ReturnLength);

		if(NT_SUCCESS(Status) && (ObjectNameInfo->Name.Length != 0))
			return( ObjectNameInfo );
		else 
		{
			ExFreePool(ObjectNameInfo);
			return NULL;
		}
	}

	return NULL;
}