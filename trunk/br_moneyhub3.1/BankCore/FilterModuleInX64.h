/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  CFilterModuleInX64
 *      ˵����  һ���ں˹ҹ��ࡣ
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2011.02.12	�Ϻ�	��ʼ�汾
 *	
 *-----------------------------------------------------------*
 */

#pragma once

enum  RETURNTYPE
{
	ISBLACK = 0,
	ISWHITE,
	ISGRAY,
};

#define HASH64  16
struct checkModuleX64
{
	unsigned char md5[HASH64];

	bool operator <(const checkModuleX64& vi) const
	{
		return memcmp(md5, vi.md5, HASH64) < 0;
	}
};
typedef std::set<checkModuleX64> CheckModuleX64;;

class CFilterModuleInX64
{
public:
	CFilterModuleInX64(void);
	~CFilterModuleInX64(void);
public:
	/**
	*
	*/
	bool InitializeHook();

	/**
	*
	*/

private:
	static bool m_bRunOnce ;
	
};



extern CFilterModuleInX64 g_filterModuleInX64App;