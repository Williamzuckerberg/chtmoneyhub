#pragma  once

#define MY_PRO_NAME						L"MoyHubSvr"

//�ؼ��߳�
#define MY_THREAD_ID_INIT					2001 //������ݳ�ʼ��
#define MY_THREAD_DRIVER_COM				2002 //������ͨ��
#define MY_THREAD_BANK_UAC					2003 //uac
#define MY_THREAD_USBKEY_CHECK			    2004 //usbkey���
#define MY_THREAD_DLG_FUC					2005 //��ʱ�Ĺ���ʵ��
#define MY_THREAD_UI_COM					2006 //��uiͨ��
#define MY_THREAD_CATHE						2007

#define MY_ERROR_SQL_ERROR					   3000 //sqlite����
#define MY_ERROR_RUNTIME_ERROR				   3001 //sqlite�ӿ��е�runtime����