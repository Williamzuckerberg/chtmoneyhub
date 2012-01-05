#include "stdafx.h"
#include "USBControl.h"
#include "USBDevice.h"
#include "USBInfoFile.h"
#pragma once

CUSBControl* CUSBControl::m_Instance = NULL;

CUSBControl* CUSBControl::GetInstance()
{
	if(m_Instance == NULL)
		m_Instance = new CUSBControl();
	return m_Instance;
}

CUSBControl::CUSBControl()
{
}
CUSBControl::~CUSBControl()
{
}

bool CUSBControl::InitUSBInfo()
{
	//���ļ����usb����Ϣ
	CUSBInfoFileManager::GetInstance()->InitUSBManegerList(m_usbinfo);
	return true;
}

void CUSBControl::BeginUSBControl()
{
	InitUSBInfo();
	DWORD dwThreadID;
	::CloseHandle(::CreateThread(NULL, 0, _threadUSBTest, NULL, NULL, &dwThreadID));
}

DWORD WINAPI CUSBControl::_threadUSBTest(LPVOID lp)
{
	DWORD threadid = ::GetCurrentThreadId();
	CAxUSBControl axui;


	CRecordProgram::GetInstance()->RecordCommonInfo(MY_PRO_NAME, MY_THREAD_ID_TESTUSB, L"������USB����߳�");

	// ����Ҫ���usb���豸������Ϣ����������Ҫע��Ϊ���㴰��
	if(axui.Create(NULL, 0 , L"Moneyhub_USBKEY_Test", WS_OVERLAPPEDWINDOW) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	// ��Ϣѭ�����ȴ��µ�usb�������Ϣ
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return 0;
}