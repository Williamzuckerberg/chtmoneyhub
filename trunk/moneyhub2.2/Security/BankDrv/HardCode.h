#pragma once

#include "ntddk.h"

extern ULONG g_NtMapViewOfSectionID;

extern ULONG g_Offset_SegmentInSection;
extern ULONG g_Offset_ControlAreaInSegment;
extern ULONG g_Offset_FileObjectInControlArea;

extern RTL_OSVERSIONINFOW g_SysVersion;

/**
* ����ϵͳ�汾��ʼ��Ӳ����ƫ�ơ�
* return �����Ƿ�ɹ���
*/
bool InitHardCode();
