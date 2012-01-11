#pragma once
#define WM_MULTI_PROCESS_CREATE_NEW_PAGE			(WM_USER + 0x4000)

#define WM_GLOBAL_CREATE_NEW_WEB_PAGE				(WM_USER + 0x4001)
#define WM_MULTI_PROCESS_NOTIFY_AXUI_CREATED		(WM_USER + 0x4002)
#define WM_FAV_BANK_CHANGE							(WM_USER + 0x400A) // gao ���û��ղ�����ʱ
#define WM_FAV_BANK_DOWNLOAD						(WM_USER + 0x400B) // gao �û����ղ����пؼ���������
#define WM_MY_MENU_CLICKED							(WM_USER + 0x400C) // gao �û�������Զ���˵�
#define WM_USER_INFO_MENU_CLICKED					(WM_USER + 0x400D) // ��¼�����û����ĵ���¼�
#define WM_SHOW_USER_CLICKED_MENU					(WM_USER + 0x400E)
#define WM_GLOBAL_GET_EXIST_WEB_PAGE				(WM_USER + 0x4014)

#define WM_MAINFRAME_CLOSE							(WM_USER + 0x4015)

#define WM_ITEM_NOTIFY_CREATED						(WM_USER + 0x4003)
#define WM_ITEM_SET_MAIN_TOOLBAR					(WM_USER + 0x4004)
#define WM_ITEM_SET_STATUS							(WM_USER + 0x4005)
#define WM_ITEM_SET_TAB_TEXT						(WM_USER + 0x4006)
#define WM_ITEM_SET_TAB_URL							(WM_USER + 0x4007)
#define WM_ITEM_GET_AX_CONTROL_WND					(WM_USER + 0x4008)
#define WM_ITEM_CHECK_URL_REQUIRE					(WM_USER + 0x4009)
#define WM_ITEM_SET_SSL_STATE						(WM_USER + 0x4010)
#define WM_ITEM_AUTOCLOSE							(WM_USER + 0x4011)
#define WM_ITEM_SET_PAGE_PROGRESS					(WM_USER + 0x4012)
#define WM_ITEM_TOGGLE_CATECTRL						(WM_USER + 0x4013)


#define WM_TAB_AUTOCLOSE							(WM_USER + 0x4020)
//	ȡ����ȡ�˵�
#define WM_CANCEL_GET_BILL							(WM_USER + 0x4021)
#define WM_GETTING_BILL								(WM_USER + 0x4022)
// ������ȡ�˵�
#define WM_FINISH_GET_BILL							(WM_USER + 0x4023)
#define WM_SET_DISPLAYHWND							(WM_USER + 0x4024)
#define WM_AX_ACCOUNT_SELECT						(WM_USER + 0x4025) // ��ʾѡ���˵�����
#define WM_AX_GET_ALL_BILL							(WM_USER + 0x4026) // ����õ��˵�����js
#define WM_AX_SHOW_INFO_DLG							(WM_USER + 0x4027) // �����˵����м���ʾ�Ի���
#define WM_AX_END_INFO_DLG							(WM_USER + 0x4028)
#define WM_AX_CANCEL_GETBILL						(WM_USER + 0x4029) // ȡ�������
#define WM_AX_EXCEED_GETBILL_TIME					(WM_USER + 0x4030) // ��ʱ�����
#define WM_RE_GETBILL								(WM_USER + 0x4031) // �����˵����´���ҳ
#define WM_AUTO_USER_DLG							(WM_USER + 0x4032) // �򿪻�رգ��û�ע�ᡢ��½���Ի���
#define WM_UPDATE_USER_STATUS						(WM_USER + 0x4033) // �����û�״̬
#define WM_USER_INFO_CLICKED						(WM_USER + 0x4034) // ��ɫ̾�ŵ���¼�
#define	WM_SHOW_USER_DLG							(WM_USER + 0x4035) // ��ʾ�û�ע���½�ȵȶԻ���
#define WM_CHANGE_USER_DLG_NAME						(WM_USER + 0x4036) // ���Ŀ�����ƣ�JS���ã�
#define WM_AX_LOAD_USER_QUIT						(WM_USER + 0x4037) // �û��˳���¼
#define WM_CHANGE_FIRST_PAGE_SHOW					(WM_USER + 0x4038) // �����û��Ƿ��¼������ҳ��ʾ��ͬ����
#define WM_AX_CHANGE_SETTINT_STATUS					(WM_USER + 0x4039) // ֪ͨ�ں��û���������ʱ��ʾ�Ľ���
#define WM_RESEND_VERIFY_MAIL						(WM_USER + 0x403A) // �ط��ʼ�
#define WM_NOTIFYUI_CLOSE_CUR_USER					(WM_USER + 0x403B) // ֪ͨUI�رյ�ǰ�û���
#define WM_NOTIFYUI_UPDATE_USER_DB					(WM_USER + 0x403C) // ֪ͨUI���µ�ǰ�û���
#define WM_NOTIFYUI_SYNCHRO_BTN_CHANGE				(WM_USER + 0x403D) // ֪ͨUIͬ����ť������ɫ
#define WM_AX_CALL_JS_SHOW_TEXT						(WM_USER + 0x403E) // ����JS����ʾ����
#define WM_ITEM_ASK_SAVE_FAV						(WM_USER + 0x4040)
#define WM_ITEM_TEST_SAVE_FAV						(WM_USER + 0x4041)
#define WM_AX_INIT_SYNCHRO_BTN						(WM_USER + 0x4042) // ��ʼ��ͬ����ť״̬
#define WM_NOTIFYUI_CUR_USER_STOKEN					(WM_USER + 0x4043) // ֪ͨUI���µ�ǰ�û�stoken
#define WM_SET_JSPARAM       						(WM_USER + 0x4044) // ���½���


#define WM_AX_NAVIGATE								(WM_USER + 0x5000)
#define WM_AX_GET_WEBBROWSER2_CROSS_THREAD			(WM_USER + 0x5001)

#define WM_AX_GOBACK								(WM_USER + 0x5002)
#define WM_AX_GOFORWARD								(WM_USER + 0x5003)
#define WM_AX_REFRESH								(WM_USER + 0x5004)
#define WM_AX_SSLSTATUS								(WM_USER + 0x5005)


#define WM_AX_FRAME_SETALARM						(WM_USER + 0x5006)
#define WM_AX_FRAME_ADDFAV							(WM_USER + 0x5007) // ����ղ�
#define WM_AX_FRAME_CHANGE_PROGRESS					(WM_USER + 0x5008) // ���½���
#define WM_AX_TOOLS_CHANGE							(WM_USER + 0x5009) // �����¼�
#define WM_AX_MONTH_SELECT_DLG						(WM_USER + 0x500A) // ��������Ի���
#define WM_AX_CALL_JS_TABACTIVE						(WM_USER + 0x500B) // ����JS TabActivated����
#define WM_AX_FRAME_DELETEFAV						(WM_USER + 0x5011) // ȡ���ղ�
#define WM_AX_USER_AUTO_LOAD						(WM_USER + 0x5012) // �û��Զ���¼
//#define WM_FINIHS_AUTO_LOAD							(WM_USER + 0x5013) // �Զ���¼������

#define WM_CLOUDALARM								(WM_USER + 0x2100)
#define WM_CLOUDCLEAR								(WM_USER + 0x2101)
#define WM_CLOUDCHECK								(WM_USER + 0x2102)
#define WM_CLOUDNCHECK								(WM_USER + 0x2103)
#define WM_CLOUDNDESTORY							(WM_USER + 0x2104)
#define WM_RESTARTMONHUB							(WM_USER + 0x2105)

#define WM_CANCEL_ADDFAV							(WM_USER + 0x2140)// ȡ���ղ�
#define WM_AX_GET_BILL								(WM_USER + 0x2141)
#define WM_AX_CLOSE_GET_BILL						(WM_USER + 0x2142)

#define WM_USB_ADD_FAV								(WM_USER + 0x2200)
#define WM_USB_CHANGE								(WM_USER + 0x2201)
#define WM_MONEYHUB_UAC								(WM_USER + 0x2204)
#define WM_MONEYHUB_FEEDBACK						(WM_USER + 0x2210)//�ƽ�㷴��

#define WM_MYDANGEROUS                              (WM_USER + 0x2202)//��⵽Σ�գ���ʾ��ʾ


// ����̼��һЩ��������
#define MY_PARAM_END_TAG								"#" // ��������ϲ��õķָ�����������PHPͨѶ���ں˺�UI��
#define MY_TAG_LOAD_DLG									1 // ��ʾ��¼�Ի���
#define MY_TAG_REGISTER_DLG								2 // ��ʾע��Ի���
#define MY_TAG_SETTING_DLG								3 // ��ʾ���öԻ���
#define MY_TAG_REGISTER_GUIDE							4 // ��ʾע���򵼶Ի���
//#define MY_TAG_SEND_OPT								5 // ֪ͨ����������OPT��ָ����������
//#define MY_TAG_CHECK_OPT								6 // У��OPT
#define MY_TAG_INIT_PWD									7 // ��������

//#define MY_STATUE_USER_LAODED							1 // �û����ڵ�¼״̬
//#define MY_STATUE_USER_NOTLOAD							2 // �û�����δ��¼״̬

#define MY_STATUE_SHOW_DLG								1 // �򿪴���
#define MY_STATUE_CLOSE_DLG								2 // �رմ���
#define MY_STATUS_HIDE_DLG								3 // ���ش���

#define MY_USER_INFO_MENU_CLICK_MAILCHANGE				1 // �޸�����
#define MY_USER_INFO_MENU_CLICK_PWDCHANGE				2 // �޸�����
#define MY_USER_INFO_MENU_CLICK_QUIT					3 // �˳�

#define MY_MAIL_VERIFY_SUCC								"46" // ���������ͻص�ֵ����ʾ�ʼ���֤�ɹ�
#define MY_MAIL_VERIFY_ERROR							"47" // ���������ͻص�ֵ����ʾ�ʼ�δ��֤����֤ʧ��

#define REGEDIT_MONHUB_PATH								"Software\\Bank\\Setting"
#define MONHUB_GUIDEINFO_KEY							"GInfo"
#define MONHUB_GUEST_USERID								"Guest"