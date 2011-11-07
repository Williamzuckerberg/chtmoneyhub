/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  MemoryProtect.cpp
*      ˵����  CPUд����������
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2010.07.03	�ܼ���
*-----------------------------------------------------------*
*/

#include "MemoryProtect.h"

VOID DisableWriteProtect(PULONG pOldAttr)
{
	ULONG uAttr;
	_asm
	{
		push	eax;
		mov		eax, cr0;
		mov		uAttr, eax;
		and		eax, 0FFFEFFFFh;
		mov		cr0, eax;
		pop		eax;
	};

	*pOldAttr = uAttr;
}

VOID EnableWriteProtect( ULONG uOldAttr )
{
	_asm
	{
		push eax;
		mov   eax, uOldAttr;
		mov   cr0, eax;
		pop   eax;
	};
}