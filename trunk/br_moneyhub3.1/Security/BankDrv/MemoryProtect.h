#pragma once

#include "ntddk.h"

/**
* ȡ��д����
* @param pOldAttr ����õĿռ����ڱ���ǰ״̬��
*/
VOID DisableWriteProtect(OUT PULONG pOldAttr);

/**
* ����д����
* @param uOldAttr ����DisableWriteProtectʱ�Ӳ������ص�״̬��
*/
VOID EnableWriteProtect(ULONG uOldAttr);