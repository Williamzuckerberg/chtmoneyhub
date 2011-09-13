#pragma once
#include "SkinProperty.h"
#include "TuoImage.h"


enum BorderEffect
{
	Border_OwnerDrawn,		// �Ի�߿�
	Border_SystemBorder,	// ʹ��ϵͳ�Դ��߿�
	Border_AdvanceAero		// ʹ������ie8����ȫ͸���߿�
};


// ע���������������ܽ����о�ֻ��һ��

class CSkinLoader
{

public:

	static void Initialize();

	void LoadSkinProperty();

private:

	CSkinLoader();


	CSkinProperty m_propertyReader;

public:

	BorderEffect m_eDisplayBorder;		// ���ڱ߿�ģʽ
	int m_iNeedDuplicateFrame;			// �Ƿ���Ҫ����frame��0=�ݲ���Ҫ��1=��Ҫ��2=�Ѿ����ƹ���������Ҫ��
	int m_iRebarAeroHeight;				// rebar����aeroЧ����ʾ����������>10��ʾ��ʾ����rebar��
	bool m_bStatusbarAero;				// statusbar����aeroЧ����ʾ
	bool m_bTabAero;					// tabbar���ֵ�aeroЧ����ʾ


	static bool sm_bIsSupportAreoEffect;
	static bool sm_bIsVista;
	static bool IsSupportAeroEffect() { return sm_bIsSupportAreoEffect; }
	static bool IsVista() { return sm_bIsVista; }
	static void RefreshAeroStatus();
};


extern CSkinLoader *g_pSkinLoader;

inline CSkinLoader* sl()
{
	return g_pSkinLoader;
}
