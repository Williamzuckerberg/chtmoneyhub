#include "../UIControl/BaseClass/TuoToolbar.h"
#include "../Skin/TuoImage.h"
#include <string>
using namespace std;

// ��barctrlΪ����İ�ť�࣬������������Ϣ���͸�������
// �ӿڵ���˳��Ϊ
class CTipsButton : public CTuoToolBarCtrl<CTipsButton>
{
public:
	CTipsButton();

	DECLARE_WND_CLASS_NODRAW(_T("MH_TipsButton"))

public:
	// ���ذ�ťͼ���
	void Refresh();
	// ��Ϊ������Ҫ���ص�
	int GetWidth() const { return m_bitmap.GetWidth() / m_statenumber; }
	int GetHeight() const { return m_bitmap.GetHeight(); }
	
	void DrawButton(CDCHandle dc, const RECT &rc, UINT uButtonStatus) const;


private:
	CTuoImage m_bitmap;
	UINT m_msg;
	int m_statenumber;


public:
	// ���ð�ť���ص�ͼƬ�͸�ͼƬ����ʾ��״̬����
	void SetBitmapFileName(wstring fname,int statenumber);
	// ������ť�ĺ�������Ҫ��ø����ھ������Ϣ
	void CreateButton(HWND hParent,UINT msg);

	// override
	void DrawBackground(HDC hDC, const RECT &rc);
	void DrawButton(HDC hDC, int iIndex, const RECT &rc, UINT uCurrentStatus);

	void SetButtonState(bool bEnabled);
protected:
	BEGIN_MSG_MAP_EX(CTipsButton)
		CHAIN_MSG_MAP(CTuoToolBarCtrl<CTipsButton>)
		MSG_WM_CREATE(OnCreate);
		TUO_COMMAND_CODE_HANDLER_EX(NM_CLICK, OnClick)
	END_MSG_MAP()


	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnClick(int nIndex, POINT pt);
};