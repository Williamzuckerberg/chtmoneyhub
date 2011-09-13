/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  CollectInstallInfo.h
 *      ˵����  �ռ���װ������з�����Ϣ����������ļ���
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.19	������	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#include "../HardwareID/genhwid.h"
#include "../HardwareID/cpuid.h"
#include "../HardwareID/SMBiosStructs.h"
#include <string>
using namespace std;

/*
���ݣ�data����
���������Ϣ��
	Ӳ��ID				terminal_id
����ϵͳ����		system_name
����ϵͳ�汾		system_version
����ϵͳ����		system_lang
IE������汾		ie_version
ɱ���������		antivirus_name
����ǽ�������		firewall_name(������û��)
�����������		
Ӳ��������Ϣ
CPU Name��			cpu_name
System Manufacturer		product_dmi
Product Name			product_name 
�����ڴ��С			memory_size

*/
class CCollectInstallInfo
{
public:
	CCollectInstallInfo(void);
	~CCollectInstallInfo(void);
public:
	// ��ð�װʱ��������Ϣ
	string GetInstallInfo();
private:
	// ���Ӳ��ID�ĺ�����Ӳ��idΪ��˾���Զ��壬�ο������ĵ���
	void GetTerminal_id();
	// ���ϵͳ���Ƽ��汾�ĺ���
	void GetSystem_nameAversion();
	// ���ϵͳ����
	void GetSystem_lang();
	// ���IE�汾
	void GetIe_version();
	// ���ɱ���������
	void GetAntivirus_name();

	// ���CPU����
	void GetCpu_name();
	// ��ò�Ʒ�������̺��ͺ�����
	void GetProduct_dmiAname();
	// ���ϵͳ�ڴ��С
	void GetMemory_size();
	// ��wstring���ͱ���ת��Ϊstring����
	string ws2s(const wstring& ws);

	// ��¼xml��Ϣ�ĳ�Ա����
	string m_sXmlInfo;
};
