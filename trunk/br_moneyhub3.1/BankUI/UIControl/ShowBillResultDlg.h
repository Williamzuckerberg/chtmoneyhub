#include "../resource/Resource.h"
#include "../Util/ThreadStorageManager.h"
#include "atlctrlx.h"
#include "atlwinex.h"
#include "../UIControl/FloatingTip/Structs.h"
#include "../Skin/SkinManager.h"
#include "../UIControl/AltSkinClasses.h"
#include <list>
using namespace std;

typedef struct MsgBillResult
{
	wstring info;		//
	bool	result;		// ��ʾ�����˵��Ľ��
}MSGBILLRESULT, *LPMSGBILLRESULT;

#define  LINE_HEIGHT 20
class CShowBillResultDlg : public CDialogImpl<CShowBillResultDlg>, public CDialogSkinMixer<CShowBillResultDlg>
{
public:
	CShowBillResultDlg(IN OUT list<LPMSGBILLRESULT>* pInfoNode)
	{
		m_pInfoNode = pInfoNode;
	}
	~CShowBillResultDlg()
	{

	}

public:
	enum { IDD = IDD_DLG_BILL_SELECT };

	BEGIN_MSG_MAP(CShowBillResultDlg)
		CHAIN_MSG_MAP(CDialogSkinMixer<CShowBillResultDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_fontText.CreateFont(15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma"));
		m_fontHelp.CreateFont(13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma"));

		m_crTextColor = RGB(16, 93, 165); // ������ɫΪ��ɫ
		m_crSuTextColor = RGB(0, 128, 0);//�ɹ�����ɫ
		m_crFaTextColor = RGB(163, 21, 21);//ʧ�ܵ���ɫ

		int lsize = 0;

		if(m_pInfoNode != NULL)
			lsize = m_pInfoNode->size();
		m_bShowWarnInfo = false;
		list<LPMSGBILLRESULT>::iterator ite = m_pInfoNode->begin();
		for(;ite != m_pInfoNode->end();ite ++)
		{
			if((*ite)->result == false)
			{
				m_bShowWarnInfo = true;
				break;
			}
		}

		//�����С
		CRect rect;
		GetClientRect(&rect);

		if(m_bShowWarnInfo)
			rect.bottom = rect.top + 20 * lsize + 100 + 50;//���Ϊ452����Ϊ��Դ�ļ���СΪ452
		else
			rect.bottom = rect.top + 20 * lsize + 100;//���Ϊ452����Ϊ��Դ�ļ���СΪ452
		if(rect.bottom > 452)
			rect.bottom = 452;
		rect.right = rect.left + 400;		

		::SetActiveWindow(m_hWnd);
		SetWindowPos(HWND_TOP, 0, 0, 0, 0,SWP_NOSIZE);		
		SetWindowText(L"�����˵����");

		::ShowWindow(GetDlgItem(IDCANCEL),SW_HIDE);//��ȡ����ť����

		ApplyButtonSkin(IDOK);

		// ȷ����ť��λ
		CRect rectBtn;
		::GetClientRect(GetDlgItem(IDOK), &rectBtn);
		::MoveWindow(GetDlgItem(IDOK), rect.Width() / 2 - rectBtn.Width() / 2, rect.Height() - rectBtn.Height() - 10, rectBtn.Width(), rectBtn.Height(), true);


		MoveWindow(0, 0, rect.Width(), rect.Height());

		CenterWindow(GetParent());

		return 0;
	}
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		RECT rcClient;
		GetClientRect(&rcClient);

		dc.SelectFont(m_fontText);
		dc.SetBkMode(TRANSPARENT);

		long nPos = 50;
		if(m_pInfoNode != NULL)
		{
			int i = 0;
			list<LPMSGBILLRESULT>::iterator ite = m_pInfoNode->begin();
			for(;ite != m_pInfoNode->end();ite ++)
			{
				RECT rctext(rcClient);
				
				rctext.top = i * 20 + 50;
				rctext.bottom = rctext.top + 20;
				nPos = rctext.bottom;

				RECT lRctext(rctext), rRctext(rctext);

				lRctext.right = 280;
				lRctext.left = 80;

				rRctext.right = 330;
				rRctext.left = 280;
				dc.SetTextColor(m_crTextColor);
				dc.DrawText((*ite)->info.c_str(), -1, (LPRECT)&lRctext, DT_WORD_ELLIPSIS | DT_SINGLELINE | DT_LEFT | DT_NOPREFIX);
				if((*ite)->result == true)
				{
					dc.SetTextColor(m_crSuTextColor);
					dc.DrawText(L"�ɹ�", -1, (LPRECT)&rRctext, DT_WORD_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CENTER);
				}
				else
				{
					dc.SetTextColor(m_crFaTextColor);
					dc.DrawText(L"ʧ��", -1, (LPRECT)&rRctext, DT_WORD_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CENTER);
				}
				i ++;
			}
		}

		if(m_bShowWarnInfo == true)
		{	
			nPos += 10;
			dc.SelectFont(m_fontHelp);
			dc.SetTextColor(m_crTextColor);
			RECT rcHelpText1(rcClient);
			rcHelpText1.top = nPos;
			rcHelpText1.bottom = nPos + 15;
			rcHelpText1.left = 40;
			rcHelpText1.right = 360;

			RECT rcHelpText2(rcHelpText1);
			rcHelpText2.top = nPos + 15;
			rcHelpText2.bottom = nPos + 30;


			wstring helpinfo1 = L"ע�����֡���������������Ϊ��������״�����ѣ�������";
			wstring helpinfo3 = L"��������ʧ���������������������������������������";
			wstring helpinfo2 = L"��ϵͳ�����������Գ������µ�¼";

			dc.DrawText(helpinfo1.c_str(), -1, (LPRECT)&rcHelpText1, DT_WORD_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CENTER);
			dc.DrawText(helpinfo2.c_str(), -1, (LPRECT)&rcHelpText2, DT_WORD_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CENTER);
			dc.SetTextColor(m_crFaTextColor);
			dc.DrawText(helpinfo3.c_str(), -1, (LPRECT)&rcHelpText1, DT_WORD_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_CENTER);
		}


		return 0;
	}
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDOK);
		return 0;
	}

private:
	list<LPMSGBILLRESULT>*	m_pInfoNode;
	bool				m_bShowWarnInfo;

	std::string			m_strDlgText;
	std::string			m_strTitleText;

	CFont m_fontText;//�������ֵ�����
	CFont m_fontHelp;//�������ֵ�����
	DWORD m_crTextColor;//��ͨ����ɫ
	DWORD m_crSuTextColor; // �ɹ�Ϊ��ɫ
	DWORD m_crFaTextColor; // ʧ��Ϊ��ɫ
};