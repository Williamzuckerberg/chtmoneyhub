#pragma  once

#define MY_PRO_NAME						L"MoyHubIE"

//�ؼ��߳�
#define MY_THREAD_ID_INIT					2003 //������ݳ�ʼ��
#define MY_THREAD_IE_STATE					2004 //IE���
#define MY_THREAD_BANK_DOWNLOAD				2005 //���ذ�װ���пؼ�����
#define MY_THREAD_USBKEY_CHECK			    2006 //usbkey���ذ�װ����
#define MY_THREAD_IE_EXTERNEL				2007 //ie��չ�ӿڹ���
#define MY_THREAD_GET_BILL					2008 //��ȡ�˵����

// gao
#define MY_ERROR_PRO_CORE                      L"moneyhubCore"
#define MY_ERROR_PRO_UI                      L"moneyhubUI" // listmanager��Ҫ�������ں˻���UI
enum {
	// URL����ʧ��
	ERR_INTOPEN = 3000,  // internet open error
	ERR_INTCONNECT, // internet connect error
	ERR_INTOPENREQ, // internet open request error
	ERR_INTSENDREQ, // internet send request error
	ERR_INTADDREQHEAD, // internet add requerst header error
	ERR_INTQUREYINFO,// internet query info error
	ERR_INTFILENOTFOUND, // �����ļ�������
	ERR_INTQUERYDATAAVAILABLE, // internte query data available error
	ERR_INTREADFILE, // internet read file error
	ERR_INTWRITEFILE, // internet write file error
	ERR_OUTOFTIME, // out of time error
	ERR_CATCH,
	ERR_SETOPTION,

	ERR_RENAMEFILE,
	ERR_CREATEPROCESS,
	ERR_CREATEFILE, // create file error
	ERR_WRITEFILE, // write file error
	ERR_SETFILEPOINTER, // set file pointer error

	ERR_SETUP_CAB_OUTOFTIME, // ��װCAB����װ��ʱ
	ERR_SETUP_EXE_OUTOFTIME, // ��װexe�ļ���ʱ
	ERR_READ_XML_CTRL, // ��ȡ���пؼ���XML�ļ�����
	ERR_READ_XML_USBKEY, // ��ȡUSBKEY��XML�ļ�����
	ERR_DOWNLOAD_XML_CTRL, // �������пؼ���XML�ļ�����
	ERR_DOWNLOAD_XML_USBKEY, // ����USBKEY��XML�ļ�����
	ERR_DOWNLOAD_URL_NOFOUND_CTRL, // û�ж�ȡ�����пؼ�����������
	ERR_DOWNLOAD_URL_NOFOUND_USBKEY, // û�ж�ȡ��USBKEY����������
	ERR_CHECK_DOWNLOAD_FILE,
	ERR_SETUP_DOWNLOAD_FILE,

	ERR_XML_FILE,
	ERR_UNHANDLE_EXCEPT, // δ������쳣

};