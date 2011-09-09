#pragma once
#include "../TuoImage.h"

#define MENU_MARGIN_DELTA			2

#define MENU_SEP_HEIGHT				9


class CMenuSkin
{

public:

	CMenuSkin(CSkinManager *pSkin);

	void Refresh();

	int GetMenuHeight()				{ return m_imgMenuBack.GetHeight(); }
	int GetMenuMarginDelta() const	{ return MENU_MARGIN_DELTA; }
	static int GetMenuPadding()		{ return 2; }


	// ���߿򱳾����Ϳյģ�, WindowDC, (0,0)Ϊ�˵����Ͻ�
	void DrawMenuBorder(CDCHandle dc, HDC dcBack, const RECT &rect, bool bJustDrawBorder, bool bEraseBg);
	// ����߶�����, ClientDc, (0,0)�������˵����Ͻ���һ������
	void DrawMenuCoreBackgroundWithClientDC(CDCHandle dc, HDC dcBack, const RECT &rcClient);
	// �����в˵��м�ķָ���
	void DrawColumnSepLine(CDCHandle dc, const RECT &rcClient, int iLineX);

	void DrawMenuItemSelectionBack(CDCHandle dc, const RECT &rect);
	void DrawMenuItemText(CDCHandle dc, UINT uMenuRes, WORD wMenuID, const CString &lpszMenuString, const RECT &rect, CBitmapHandle hBitmap, CBitmapHandle hBitmapMask, bool bEnabled, bool bBold);
	void DrawMenuSep(CDCHandle dc, const RECT &rect);

	void DrawMenuNonClient(CDCHandle dc, const RECT &rect);

private:

	CTuoImage m_imgMenuBack;
	CTuoImage m_imgMenuIcon;
	CTuoImage m_imgMenuSep;
};
