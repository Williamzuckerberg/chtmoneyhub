/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  MapViewOfSection.cpp
*      ˵����  ���˹��ܿ�ܡ�
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2010.07.03	�ܼ���
*-----------------------------------------------------------*
*/

#include "ntddk.h"
#include "MapViewOfSection.h"
#include "ServiceTable.h"
#include "windef.h"
#include "QueryObjectName.h"

#include "ProcessFilter.h"
#include "FilterCache.h"
#include "md5.h"
#include "SecuHash.h"
#include "BlackHash.h"
#include "HardCode.h"
#include "hookSST.h"
#include "LogSystem.h"
#include "SendDataToGUI.h"

#define WRITELOGBH   100
//////////////////////////////////////////////////////////////////////////

#define		FILE_BUFFER_SIZE	(64 * 1024) 

extern "C" POBJECT_TYPE* MmSectionObjectType;
KMUTEX	g_DispatchMutex;
LONG g_HookCounter   = 0;
LONG g_MapLock       = 0;
bool g_bPassFilter   = false;
//ͳ�ƹ���ʧЧ�������糬��__�Σ�moneyhub�˳�
ULONG   g_count=0;

typedef NTSTATUS (NTAPI *NTMAPVIEWOFSECTION)(
	IN HANDLE SectionHandle,
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN ULONG ZeroBits,
	IN ULONG CommitSize,
	IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
	IN OUT PULONG ViewSize,
	IN SECTION_INHERIT InheritDisposition,
	IN ULONG AllocationType,
	IN ULONG Protect);

static NTMAPVIEWOFSECTION NtMapViewOfSection = NULL;

//////////////////////////////////////////////////////////////////////////

/**
* �ȼ�������������ں������ڣ�ֱ�Ӿܣ�������С�
   ������ں�������������������ڰ������ڣ��޲�����
     ������ڰ������ڣ����ļ�ȫ·����hashֵ���ݵ��ϲ�
* @param filename �ļ�ȫ·����
* return ����ļ���ȫ�򷵻�true�����򷵻�false��
*/
bool CheckIsFileHashSecure(const PUNICODE_STRING filename)
{	
	/////////////////////////////////////////////�Ƚ��Ѿ����˵��ļ���
	if(IsInBlackCache(filename) == true)
		return false;

	if( MODULE_FILTERED == GetModuleFilter((ULONG)PsGetCurrentProcessId(),filename) )		
		return true;
	///////////////////////////////////////

	HANDLE hFile;
	OBJECT_ATTRIBUTES oaFile;
	InitializeObjectAttributes(&oaFile, filename, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	// ��Ȩ�޴��ļ������ʧ������Ϊ�ļ�����ȫ������false��
	IO_STATUS_BLOCK ioStatus;
	NTSTATUS status = ZwOpenFile(&hFile, GENERIC_READ, &oaFile, &ioStatus, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("VerifyModule: ZwOpenFile: %ws %08x\n", filename->Buffer, status));
		return false;
	}

	unsigned char* fileBuf = (unsigned char*)ExAllocatePoolWithTag(PagedPool, FILE_BUFFER_SIZE, 'knab');
	if(fileBuf == NULL)
	{
		ZwClose(hFile);
		return false;
	}

	// ��ȡ�ļ�hash��
	MD5_CTX md5;
	MD5Init(&md5);
	ULONG sizeAll=0;
	
	FILE_STANDARD_INFORMATION fsi;
	ZwQueryInformationFile(hFile,&ioStatus,&fsi,sizeof(FILE_STANDARD_INFORMATION),FileStandardInformation);


	while(1)
	{
		NTSTATUS status = ZwReadFile(hFile, NULL, NULL, NULL, &ioStatus, fileBuf, 
			FILE_BUFFER_SIZE, NULL, NULL);
		if(!NT_SUCCESS(status))
			break;
		if(ioStatus.Information == 0)
			break;

		sizeAll += ioStatus.Information;
		MD5Update(&md5, fileBuf, ioStatus.Information);
	}
	ExFreePoolWithTag(fileBuf, 'knab');

	unsigned char final[16];
	MD5Final(final, &md5);

	ZwClose(hFile);

	//�ڰ�����У��
//  	bool bOK = IsHashBlack(final);
// 
// 	if( bOK )
// 	{
// 		if(!IsInBlackCache(filename))
// 		{
// 			WriteSysLog(LOG_TYPE_DEBUG,L" Fileter Module :%s", filename->Buffer);
// 			AddBlackCache(filename);
// 		}
// 		return false;
// 	}
// 	else if( !IsHashSecure(final) )//���ݵ��ϲ� 
// 	{
// 		if( setData(filename->Buffer,filename->Length,final,16) )
// 			setSigned();
// 	}	
	bool bOK = IsHashBlack(final);
	bool bReturn = true;

	if( bOK )
	{
		if(!IsInBlackCache(filename))
		{
			WriteSysLog(LOG_TYPE_INFO,L" Fileter Module :%s", filename->Buffer);
			AddBlackCache(filename);
		}
		bReturn = false;
	}
	else if( !IsHashSecure(final) )//���ݵ��ϲ� 
	{
		if( !IsInstall() )
		{
			if( setData(filename->Buffer,filename->Length,final,16) )
				setSigned();

			LARGE_INTEGER  lWaitTimeOuts;
			lWaitTimeOuts.QuadPart = -10 * 1000 * 1000;

			//DbgPrint("###kernel wait event!");
WriteSysLog(LOG_TYPE_INFO,L" kernel is waitint for event signal!");
			if( STATUS_TIMEOUT == KeWaitForSingleObject(g_pEventFilterGo, Executive, KernelMode, FALSE, &lWaitTimeOuts) )
			{
				IsInstall(true);
				if( GetPIDNumber() > 2)
					bReturn = false;
				else
					bReturn = true;
			}
			else
				bReturn = g_bPassFilter;
			//DbgPrint("###kernel continue: file path : %ws is %ws \n", filename->Buffer, bReturn? L"pass":L"noPass");
WriteSysLog(LOG_TYPE_INFO,L" kernel continue: file path : %ws is %ws", filename->Buffer, bReturn? L"pass":L"noPass");
			if( bReturn )
				if( MODULE_FILTERED != GetModuleFilter((ULONG)PsGetCurrentProcessId(),filename) && (GetPIDNumber() > 2) )
					SetModuleFilter((ULONG)PsGetCurrentProcessId(), filename, true);
		}
	}	
	//
	return bReturn;
}

/**
* ����ļ��Ƿ�ΪPE�ļ�
* @param filename �ļ�·����
* return ����ļ���PE�ļ��Ļ����true�����򷵻�false��
* ע�����ڵķ����Ƚϼ򵥣�ֻ������ļ���ǰ�����ֽڣ�����ټ�鼸����
*/
bool IsPEFile(const PUNICODE_STRING filename)
{
	HANDLE hFile;
	bool bIsPEFile = true;

	OBJECT_ATTRIBUTES oaFile;
	InitializeObjectAttributes(&oaFile, filename, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	IO_STATUS_BLOCK ioStatus;
	NTSTATUS status = ZwOpenFile(&hFile, GENERIC_READ, &oaFile, &ioStatus, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		FILE_SYNCHRONOUS_IO_NONALERT);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("IsPEFile: ZwOpenFile: %ws %08x\n", filename->Buffer, status));
		return true;
	}

	USHORT PESig;
	LARGE_INTEGER offset;
	offset.QuadPart = 0;
	ZwReadFile(hFile, NULL, NULL, NULL, &ioStatus, &PESig, 2, &offset, NULL);

	if(PESig != 'ZM')
		bIsPEFile = false;

	ZwClose(hFile);
	return bIsPEFile;
}

/**
*  ���˺��ĺ���
*
*/
NTSTATUS NTAPI BkMapViewOfSection(IN HANDLE SectionHandle,
								  IN HANDLE ProcessHandle,
								  IN OUT PVOID *BaseAddress,
								  IN ULONG ZeroBits,
								  IN ULONG CommitSize,
								  IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
								  IN OUT PULONG ViewSize,
								  IN SECTION_INHERIT InheritDisposition,
								  IN ULONG AllocationType,
								  IN ULONG Protect)
{
	PVOID Section = NULL;
	ULONG bModuleFilter = MODULE_PASSED;

	// ��ȡSection Object
	NTSTATUS Status = ObReferenceObjectByHandle (SectionHandle, 0x00000004, //SECTION_MAP_READ
		*MmSectionObjectType, UserMode, (PVOID *)&Section, NULL);

	// ���ʧ��ֱ�ӷ���ϵͳ����
	if(NT_SUCCESS(Status))
	{
		// ͨ��Ӳ����ֵ�ҵ�FileObject
		PVOID Segment = *(PVOID*)((ULONG)Section + g_Offset_SegmentInSection);

		if(Segment)
		{
			PVOID ControlArea = *(PVOID*)((ULONG)Segment + g_Offset_ControlAreaInSegment);

			if(ControlArea)
			{
				PVOID FileObject = *(PVOID*)((ULONG)ControlArea + g_Offset_FileObjectInControlArea);

				// �����vista����win7ϵͳ��FileObject��Ҫ������3��bits
				if((g_SysVersion.dwMajorVersion == 6 && g_SysVersion.dwMinorVersion == 1) // Windows 7
					|| (g_SysVersion.dwMajorVersion == 6 && g_SysVersion.dwMinorVersion == 0)) // Windows Vista

					FileObject = (PVOID)((ULONG)FileObject & 0xFFFFFFF8);

				if(FileObject)
				{
					//KdPrint(("==>BkMapViewOfSection\n"));

					// ��ȡ�ļ�·��
					POBJECT_NAME_INFORMATION ObjectKernelName = BkQueryNameString((PVOID)FileObject);

					if (ObjectKernelName != NULL)
					{
						//����ļ��Ƿ���PE�ļ������������һ����飬���ǵĻ�ͨ����顣
						if (IsPEFile(&ObjectKernelName->Name))
						{
							// ����Ƿ��ǿ����ļ�hash
							if (!CheckIsFileHashSecure(&ObjectKernelName->Name))
							{
								bModuleFilter = MODULE_FILTERED;
							} 
						}

						ExFreePool(ObjectKernelName);
					}

					//KdPrint(("<==BkMapViewOfSection\n"));
				}
			}
		}

		ObDereferenceObject(Section);

		if (bModuleFilter == MODULE_FILTERED)
		{	
			//KdPrint(("Module Filtered\n"));
			return STATUS_ACCESS_DENIED;
		}
	}

	// ����ԭϵͳ����
	NTSTATUS result = NtMapViewOfSection(SectionHandle, ProcessHandle, BaseAddress,
		ZeroBits, CommitSize, SectionOffset, ViewSize, InheritDisposition,
		AllocationType, Protect);

	return result;
}



/**
*   ���Ӻ���������ģ�����
*/
NTSTATUS NTAPI MyMapViewOfSection(IN HANDLE SectionHandle,
								  IN HANDLE ProcessHandle,
								  IN OUT PVOID *BaseAddress,
								  IN ULONG ZeroBits,
								  IN ULONG CommitSize,
								  IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
								  IN OUT PULONG ViewSize,
								  IN SECTION_INHERIT InheritDisposition,
								  IN ULONG AllocationType,
								  IN ULONG Protect)
{
	NTSTATUS result;

	InterlockedIncrement(&g_HookCounter);

	if (IsProcessProtected((DWORD)PsGetCurrentProcessId()) && (LONG)ProcessHandle == 0xFFFFFFFF)
	{
		result = BkMapViewOfSection(SectionHandle, ProcessHandle, BaseAddress,
		ZeroBits, CommitSize, SectionOffset, ViewSize, InheritDisposition,
		AllocationType, Protect);
	}
	else
		result = NtMapViewOfSection(SectionHandle, ProcessHandle, BaseAddress,
		ZeroBits, CommitSize, SectionOffset, ViewSize, InheritDisposition,
		AllocationType, Protect);

	InterlockedDecrement(&g_HookCounter);

	return result;
}

/**
*  ���ù��Ӻ�ж�ع��Ӻ���
*/
VOID HookSSDT()
{
	NtMapViewOfSection = (NTMAPVIEWOFSECTION)UpdateService(g_NtMapViewOfSectionID, (PVOID)MyMapViewOfSection);
	sstHook_OpenProcess();
}

VOID UnHookSSDT()
{
	WriteSysLog(LOG_TYPE_DEBUG,L" UnHookSSDT");
	//::KeWaitForSingleObject(&g_DispatchMutex,Executive,KernelMode,FALSE,NULL);
	UpdateService(g_NtMapViewOfSectionID, (PVOID)NtMapViewOfSection);
	sstUnhook_OpenProcess();
	WriteSysLog(LOG_TYPE_DEBUG,L" FinishHookSSDT");

}

/**
*  ��鹳��״̬����
*/
bool checkHookSafe()
{
	if(g_count <= 6 )
	{   		
		if(!_checkHookSafeMapV((PVOID)MyMapViewOfSection, g_NtMapViewOfSectionID) )
		{
			UpdateService(g_NtMapViewOfSectionID, (PVOID)MyMapViewOfSection);
			g_count++;
		}
	    if(!_checkHookSafeOpenP())
		{
			sstHook_OpenProcess();
			g_count++;
		}
		return true;
	}
	else
	{
		g_count=0;
		return false;
	}	

}