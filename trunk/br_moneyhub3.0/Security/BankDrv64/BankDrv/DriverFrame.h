/************************************************************************
* �ļ�����:Driver.h                                                 
* 
* 
*************************************************************************/
#pragma once
#include "DriverDefine.h"


#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")

#define		DEVICE_NAME		L"\\Device\\MoneyHubPrt64"
#define		DOS_NAME		L"\\DosDevices\\MoneyHubPrt64"

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;	//�豸����
	UNICODE_STRING ustrSymLinkName;	//����������
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// ��������
NTSTATUS CreateDevice (IN PDRIVER_OBJECT	pDriverObject); 
VOID DDKUnload (IN PDRIVER_OBJECT pDriverObject);
NTSTATUS DDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
							IN PIRP pIrp);

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP IRP);