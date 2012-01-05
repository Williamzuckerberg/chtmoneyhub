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
		MESSAGE_HANDLER_EX(WM_AX_USER_AUTO_LOAD, UserAutoLoad)
		MESSAGE_HANDLER_EX(WM_AX_LOAD_USER_QUIT, OnLoadUserQuit)
		MESSAGE_HANDLER_EX(WM_AX_GET_ALL_BILL, OnGetAllBill)
		MESSAGE_HANDLER_EX(WM_FINISH_GET_BILL, OnSendToBillPage)
		MESSAGE_HANDLER_EX(WM_RESEND_VERIFY_MAIL, OnResendVerifyMail)

		MESSAGE_HANDLER_EX(WM_TIMER, OnTimer)
		HANDLE_TUO_COPYDATA()
	END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// �����µ�ҳ��ĺ���
	LRESULT OnCreateNewPage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCancelAddFav(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT UserAutoLoad(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLoadUserQuit(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnGetAllBill(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendToBillPage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnExit(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnResendVerifyMail(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnShowUserDlg(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void CheckDownloadFile(void);

protected:
	// CBkInfoDownloadManager* m_pBkDownloadManager;
	HWND m_hTag;
	char *m_pbilldata;
	static DWORD WINAPI _threadInit(LPVOID lp);
	void CheckSystemTime(void);
public:
	static void UpdateUserInfo(const char* pStoken, const char* pMail, const char* pUserId, const int nUserStatus, bool bChangeMail, char* pVerifyMail = NULL);
};
