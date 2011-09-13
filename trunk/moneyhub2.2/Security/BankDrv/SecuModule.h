#pragma once

//////////////////////////////////////////////////////////////////////////

/**
* ��Ӱ�ȫ�ļ�·��
* @param filepath �ļ�·����
* @param length �ļ�·�����ȡ�
* return �����Ƿ���ӳɹ���
*/
bool AddSecurePath(const WCHAR* filepath, ULONG length);

/**
* ��Ӱ�ȫ�ļ�·��
* return �����Ƿ�����ɹ�
*/
bool ClearSecurePaths();


/**
* ����ļ��Ƿ�ȫ
* @param filepath �ļ�·����
* return ����ļ��ǰ�ȫ���򷵻�true�����򷵻�false��
*/
bool IsSecurePath(PUNICODE_STRING filepath);