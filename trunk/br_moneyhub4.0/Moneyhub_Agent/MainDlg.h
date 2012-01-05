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
#define ICON_WIDTH					48
#define ONEPAGENUM			3
#define NULLDATA			-1
enum ePageType
{
	eNormalPage,
	eSettingPage
};
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

		MSG_WM_LBUTTONDOWN(OnLButtonDown)

		COMMAND_ID_HANDLER(IDC_OK, OnOK)
		COMMAND_ID_HANDLER(IDC_CANCEL, OnCancel)// �����ȡ��
		COMMAND_ID_HANDLER(IDC_PRE, OnPre)
		COMMAND_ID_HANDLER(IDC_NEXT, OnNext)
		COMMAND_ID_HANDLER(ID_CLOSE, OnHide)

		MESSAGE_HANDLER(WM_NCPAINT,OnNcPaint)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)

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

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnFeedBack(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnPre(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//�رհ�ť��
	LRESULT OnHide(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void OnLookUp(wstring mail);
	void NoTips();//���ղ�������

	//LRESULT OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnNcPaint(WORD /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void OnLButtonDown(UINT nFlags, CPoint point) ;
	void OnLButtonUp(UINT nFlags, CPoint point) ;
	void OnMouseMove(UINT nFlags, CPoint point) ;

	BOOL OnEraseBkgnd(CDCHandle dc);

	bool CheckNetState();
	

protected:
	void CloseDialog(int nVal);
private:
	int m_totalTipNum;			// ��ʾ����
	int m_iPageNum;				// Ӧ����ʾ����ҳ��
	std::vector<DATUSEREVENT>		m_event; // ��õ��������˺����е��¼���ԭʼ����
	std::vector<DATUSEREVENT>		m_eventcopy; // ��õ��������˺����е��¼�����ʱ�ļ������ڸ�������
	int								m_iteCurEvent[3];//ָ��ǰ�Ķ�����ݣ����ָ��NULLDATA��Ϊû������
	void FetchData();
	void RefreshData();

	ePageType m_iCurrentPageType;		// ��¼��ǰ��ʾ��������࣬Ŀǰ��Ϊ����ҳ��������ʾҳ
	int m_iCurrentPage;			// ��ǰ��ʾ��ҳ��
	int m_iTipsNum;				// Ҫ��ʾ����������
	int m_iCurrentTipsNum;		// ��ǰҳ��ʾ����������

	void ReCalculateParam();

	CRect m_rcTotalInfo;		// �м����ʾ����
	CRect m_rcSetting;			// ����
	CRect m_rcNoTips;			// ���ղ�������
	CRect m_rcNumShow;			// ��ʾҳ���л�
	bool  m_bMouseOn[3];		// ��¼����Ƿ�����3��ͼ����
	bool  m_bOnSettingRect;		// 
	bool  m_bOnNoTipsRect;

	CSkinCheckBox m_ckBox[3];		// 3��checkbox��ť
	CSkinButton m_btNext;		// ��һҳ��ť
	CSkinButton m_btPre;		// ǰһҳ��ť
	CSkinButton m_btOK;			// ȷ����ť
	CSkinButton m_btCancel;		// ȡ����ť
	CSkinButton m_close;		// �رհ�ť

	CPen m_colorEdge;			// �������ݱ߿�ĵ�ɫ
	CBrush m_colorBack;			// ����ɫ
	CBrush m_colorTipBk;		// �м���ʾ����ĵ�ɫ
	CBrush m_colorMouseMoveBk;	// �м���껬����ʱ�����ɫ
	CPen m_colorMouseMoveLine;  // �м���껬��ʱ���м�������ߵĵ�ɫ
	CPen m_colorInnerEgde;		// ����߿����ɫ

	CIcon m_icoMainIcon;			// ico
	bool bTrackLeave;				// (Ҳ����Ϊ��Ա����)
	bool m_bShouldShow;				// �Ƿ�Ӧ����ʾ
	bool m_bTodayShow;
	int m_timer;					// ����������������뿪��ʱ��

	CTuoImage m_imgCloseButton;		// �رհ�ť�ı���
	CTuoImage m_imgPreButton;		// ǰһҳͼƬ
	CTuoImage m_imgNextButton;		// ��һҳ
	CTuoImage m_imgComButton;		// ͨ�ð�ť��ȡ����ȷ��
	CTuoImage m_imgCheckBox;		// checkbox��ͼƬ

	HFONT m_TitleFont;				// ��������
	HFONT m_CommonEngFont;			// ��ͨӢ������
	HFONT m_CommonFont;				// ��ͨ����
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
