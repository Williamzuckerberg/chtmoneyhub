#pragma once

#include "windows.h"
#include <string>

#define			DRIVER_NAME				"MoneyHubPrt"
#define         DRIVER_NAMEX64          "MoneyHubPrt64"

class CDriverLoader{
	std::string	m_DriverPath;
	std::string m_CurrentPath;
	static std::string m_drivername;
	HANDLE		m_hDriver;

public:
	CDriverLoader();

	BOOL	InitializeDriver() const;

	BOOL	InstallDriver() const;
	BOOL	CheckIsDriverInstalled() const;
	BOOL	RemoveDriver() const;

	BOOL	LoadDriver() const;
	BOOL	CheckIsDriverLoaded() const;
	BOOL	UnloadDriver();

	BOOL	StartDriver();
	BOOL	StopDriver();

	HANDLE	GetDriver(bool isUnload=false);
	BOOL	CloseDriver();
	bool	CheckDriver();//�������״̬�����2����


	static  bool    m_showMessOnce;
	static bool	CheckDriverImagePath(bool bNotSilent=true);

private:
	static void InitialDriverName();

	/**
	�жϵ�ǰϵͳ�汾
	*/
	BOOL isX64() const;
};
