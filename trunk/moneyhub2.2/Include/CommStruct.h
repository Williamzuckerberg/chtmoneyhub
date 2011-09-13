#pragma once
#include <string>


#define ICON_DEFAULT_PAGE			(HICON)0
#define ICON_HOME					(HICON)-1


//////////////////////////////////////////////////////////////////////////

// ������ö�˵�ˡ���������������
// ����CreateNewWebPageData�Ǳ䳤�ģ������¿��ܣ�
// 1. sizeof(CreateNewWebPageData) + sizeof(URL)
// 2. sizeof(CreateNewWebPageData) + sizeof(Search Text)
class CTabItem;

struct CreateNewWebPageData
{
	DWORD dwSize;			// ��dwSize=0��˵��lpszURLָ�����һ����ʵ��URL�ַ�����ַ������lpszURL��һ����ǣ�ָ��ṹ���ĩβ���ݶ�
	int iCategory;

	LPCTSTR lpszURL;
	BOOL bShowImmediately;

	// �û���ʱ�滻������
	HWND hAxControlWnd;
	CTabItem *lpTabItemData;
	HWND hFrame;			// ��hAxControlWnd�Ƶ����frame���棬���Ϊ�ձ�ʾ�´���һ��frame
	HWND hChildFrame;		// ��hAxControlWnd�Ƶ����ChildFrame���ұߣ����Ϊ�ձ�ʾ�Ƶ������

	HWND hCreateFromChildFrame;		// ͨ��������Ӵ�������ҳ�棬�������Ϊ������ӵ��Ǹ�CChildFrame
	HWND hCreateFromMainFrame;		// �����´��ڣ����¼ԭ���Ǵ��Ǹ�������

	int iNewPosition;		// �½���ǩʱ���õ�λ�ã�0��ʾĬ�ϣ������������ļ���ȷ������-1��ʾ�ŵ�ĩβ
	int iWeight;			// Ȩ�أ������������ҳ��ķ�����Ҫ��

	bool bNoClose;
};
