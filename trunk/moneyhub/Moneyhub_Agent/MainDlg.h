#include <atlctrlx.h>
#include <string>
#include "../BankData/BankData.h"
#include "Security/Security.h"
#include "Skin/AltSkinClasses.h"
#include <Dbt.h>
#pragma once
using namespace std;
#define WM_SHNOTIFY WM_USER + 0x101f//֪ͨ�ں˽������豸usb����

#define WM_FEEDBACK WM_USER + 0x102f //��֪ͨ���췴������������

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
	public CMessageFilter, public CIdleHandler
{
public:
	CMainDlg();

public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		COMMAND_ID_HANDLER(ID_CLOSE, OnCancel)
		COMMAND_ID_HANDLER(IDC_LOOKUP, OnLookUp)
		COMMAND_ID_HANDLER(IDC_NOTIPS, OnNoTips)
		MESSAGE_HANDLER(WM_MOUSELEAVE,OnMouseLeave)
		MESSAGE_HANDLER(WM_NCPAINT,OnNcPaint)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER_EX(WM_DEVICECHANGE, OnDeviceChange)
		MESSAGE_HANDLER(WM_MONEYHUB_FEEDBACK, OnFeedBack)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMouseLeave(WORD /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnFeedBack(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	
	LRESULT OnLookUp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNoTips(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnNcPaint(WORD /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void OnMouseMove(UINT nFlags, CPoint point);
	//LRESULT OnCancel(WORD /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void OnLButtonDown(UINT nFlags, CPoint point) ;

	BOOL OnEraseBkgnd(CDCHandle dc);

	bool CheckNetState();

protected:
	void CMainDlg::CloseDialog(int nVal);
private:
	HDEVNOTIFY      m_NotifyDevHandle;
	wstring m_tipnum;
	CSkinButton m_show;
	CSkinButton m_notips;
	CSkinButton m_close;
	bool bTrackLeave; //(Ҳ����Ϊ��Ա����)
	bool m_bShouldShow;//�Ƿ�Ӧ����ʾ
	bool m_bTodayShow;
	bool m_bShowNow;//�����������Ѹ�������¼��ǰ�Ƿ�Ӧ������
	int m_timer;//����������������뿪��ʱ��

	CTuoImage m_imgBackGround;	//����ͼƬ
	CTuoImage m_imgDlgButton;	//��ť�ı���
	CTuoImage m_imgCloseButton; //�رհ�ť�ı���
	HFONT m_Font;
	bool m_Netstat;//��¼����״̬


	void DrawBack(CDCHandle dc, const RECT &rect);
	void ShowPop();
	bool IsVista();
	void CheckShow();
	void AutoUpdate();
	BOOL IsAutoRunUpdate();
	bool m_delayflag;
	int PostData2Server(string hid, string sn, wstring file);

	SYSTEMTIME m_lastTime;
	__int64  TimeDiff(SYSTEMTIME  left,SYSTEMTIME  right);
};
