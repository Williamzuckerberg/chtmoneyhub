
#include "version.inc"
/*
#define ProductVersion_Major		2
#define ProductVersion_Minor		0
#define ProductVersion_Release		0
#define ProductVersion_Build		1001
*/

#define FileVersion_Major			ProductVersion_Major
#define FileVersion_Minor			ProductVersion_Minor
#define FileVersion_Release			ProductVersion_Release
#define FileVersion_Build			ProductVersion_Build


#define ProductName					"�ƽ��"
#define CompanyName					"���ź�ͨ�Ƽ����޹�˾"
#define LegalCopyright				"(C)2010 ���ź�ͨ�Ƽ����޹�˾, ��Ȩ����."
#define Comments					"�ƽ��"

#define FileDescription_setask				"�ƽ��"
#define FileName_setask						"�ƽ��"

#define FileDescription_dynamark			"dynamark.exe"
#define FileName_dynamark					"dynamark.exe"

#define FileDescription_SogouExplorer		"�ƽ��"
#define FileName_SogouExplorer				"�ƽ��"

#define FileDescription_CmdLineParser_DLL	"CmdLineParser.dll"
#define FileName_CmdLineParser_DLL			"CmdLineParser.dll"

#define FileDescription_Common_DLL			"SECommon.dll"
#define FileName_Common_DLL					"SECommon.dll"

#define FileDescription_Dialog_DLL			"Dialog.dll"
#define FileName_Dialog_DLL					"Dialog.dll"

#define FileDescription_MainUI_DLL			"MoneyHub.exe"
#define FileName_MainUI_DLL					"MoneyHub.exe"

#define FileDescription_QuickSearch_DLL		"QuickSearch.dll"
#define FileName_QuickSearch_DLL			"QuickSearch.dll"

#define FileDescription_Trash_DLL			"Trash.dll"
#define FileName_Trash_DLL					"Trash.dll"

#define FileDescription_UserCenter_DLL		"UserCenter.dll"
#define FileName_UserCenter_DLL				"UserCenter.dll"

#define FileDescription_Downloader_DLL		"Downloader.dll"
#define FileName_Downloader_DLL				"Downloader.dll"

#define FileDescription_DialogCore_DLL		"DialogCore.dll"
#define FileName_DialogCore_DLL				"DialogCore.dll"

#define FileDescription_Snapshoter_DLL		"Snapshoter.dll"
#define FileName_Snapshoter_DLL				"Snapshoter.dll"

#define FileDescription_TridentCore_DLL		"TridentCore.dll"
#define FileName_TridentCore_DLL			"TridentCore.dll"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// ���������� TuotuoXP ά��, �����޸�

#define ToString(v)				#v
#define VerJoin(a, b, c, d)		ToString(a.b.c.d)


#define FileVersion_All			VerJoin(FileVersion_Major, FileVersion_Minor, FileVersion_Release, FileVersion_Build)
#define ProductVersion_All		VerJoin(ProductVersion_Major, ProductVersion_Minor, ProductVersion_Release, ProductVersion_Build)
