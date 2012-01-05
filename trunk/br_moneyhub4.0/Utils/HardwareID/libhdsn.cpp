
#include <windows.h>
#include "libhdsn.h"


/*DWORD MapSerialToDword(char *serial)
{
	DWORD result = 0;
	char *p = serial + lstrlen(serial) -1 ;
	for (int i=0;i<5 && p >= serial ;i++,p--)
	{
		unsigned int c = (unsigned int)(*p);
		if (c >='0' && c<='9')
			c = c - '0' ;   // Сд��ĸռ�� 0-9
		else if (c >='A' && c <= 'Z')
			c = c - 'A' + 10 ;  // ��д��ĸռ�� 10-36
		else 
			c = 0; // ���˴�д��ĸ�����ֵĶ���ֵΪ 0 
		result += c;
		result <<= 6; // 6 bit 
	}
	return result;
}*/


bool GetFirstHDSerial(char *buf, size_t len)
{
	OSVERSIONINFO verinfo;
	verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&verinfo);
	switch(verinfo.dwPlatformId) {
	case VER_PLATFORM_WIN32_NT:
		if (GetFirstHDSerial_NT_PhysicalDriveAdmin(buf,len))
			return true;
		if (GetFirstHDSerial_NT_PhysicalDriveZeroRight(buf,len))
			return true;
		if (GetFirstHDSerial_NT_IDEasSCSI(buf,len))
			return true;
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		if (GetFirstHDSerial_9X_SMART(buf,len))
			return true;
#ifdef USE_PORT_DRIVER
		if (GetFirstHDSerial_9X_Ports(buf,len))
			return true;
#endif 
		break;
	default:
		break;
	}
	return false;
}