/**
*-----------------------------------------------------------*
*  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
*    �ļ�����  AxUI.h
*      ˵����  �����߳̽��̹�����
*    �汾�ţ�  1.0.0
* 
*  �汾��ʷ��
*	�汾��		����	����	˵��
*	1.0.0	2010.11.17	���ź�ͨ
*-----------------------------------------------------------*
*/
#include "BankData\\BkInfoDownload.h"
#include "Message.h"
#pragma once


class CAxUI : public CWindowImpl<CAxUI>
{

public:
	#define _SETUP_FINISH		200
	#define _DOWNLOAD_FINISH	100
	
	CAxUI();
	// �����رղ���ĺ���
	void StartQuittingSeTask();

	DECLARE_WND_CLASS(_T("MH_TuoAxUI"))

	BEGIN_MSG_MAP_EX(CAxUI)
 		MSG_WM_CREATE(OnCreate)
		MESSAGE_HANDLER_EX(WM_MAINFRAME_CLOSE, OnExit)

		MESSAGE_HANDLER_EX(WM_MULTI_PROCESS_CREATE_NEW_PAGE, OnCreateNewPage)
		MESSAGE_HANDLER_EX(WM_CANCEL_ADDFAV, OnCancelAddFav)
		//MESSAGE_HANDLER_EX(WM_FAV_BANK_DOWNLOAD, OnDownLoadBankInfo)
		HANDLE_TUO_COPYDATA()
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// �����µ�ҳ��ĺ���
	LRESULT OnCreateNewPage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCancelAddFav(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnDownLoadBankInfo (UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnExit(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void CheckDownloadFile(void);

protected:
	// CBkInfoDownloadManager* m_pBkDownloadManager;
	HWND m_hTag;
	static DWORD WINAPI _threadInit(LPVOID lp);
};
