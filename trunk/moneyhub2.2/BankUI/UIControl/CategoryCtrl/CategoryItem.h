#pragma once
#include <tuodwmapi.h>
#include "../../../Utils/ListManager/ListManager.h"
#include "../../Skin/SkinManager.h"
#include "CategoryCtrl.h"

class CTuotuoCategoryCtrl;
class CChildFrame;

class CCategoryItem
{
	friend class CTuotuoCategoryCtrl;

public:
	CCategoryItem(CTuotuoCategoryCtrl *pCateCtrl);

	// ��tab��iDeltaHeight��ʾ��chromeƤ���£���ǩ�����ϲ��ճ����ĸ߶�
	void DoPaint(CDCHandle dc, ButtonStatus eTabStatus, CategoryPosition btnPos, ButtonStatus eBtn, bool bDrawCloseBtn, int iDeltaHeight);

	const CWebsiteData* GetWebData() const		{ return m_pWebData; }

	// ����
	LPCTSTR GetText() const;
	void SetText(LPCTSTR lpszText);

	// ���
	void SetNewWidthImmediately();

	// λ��
	void SetPosImmediately(int iPos);

	// ����
	size_t GetIndex() const				{ return m_nIndex; }
	void SetIndex(size_t nIndex)		{ m_nIndex = nIndex; }

	// �Ƿ���ʾ���밲ȫɫ
	bool ShowSecureLock();
	bool ShowSecureColor();

	// �ػ�
	void Redraw()						{ m_pCateCtrl->Invalidate(); }

public:
	CTabItem *m_pSelectedItem;
	std::vector<CTabItem*> m_TabItems;

private:
	CTuotuoCategoryCtrl *m_pCateCtrl;

	int m_nIndex;
	std::tstring m_sText;

	int m_iXPos;
	int m_iWidth;

	const CWebsiteData *m_pWebData;
};
