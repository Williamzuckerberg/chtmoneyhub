/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  AES.h
 *      ˵����  AES����
 *    �汾�ţ�  1.0.0
 * 
 *  �汾��ʷ��
 *	�汾��		����	����	˵��
 *	1.0.0	2010.10.22	���ź�ͨ	��ʼ�汾

 *  ����������
 *  Visual Studio 2008
 *-----------------------------------------------------------*
 */
#pragma once

class CAES
{
	unsigned char* m_key;
	long m_keylen;

	unsigned char* m_output;
	long m_outlen;

public:
	CAES(void);
	~CAES(void);

	void SetKey(unsigned char* key, int len);

	unsigned char* Cipher(const unsigned char* input, int ilen, int& olen);
	unsigned char* DeCipher(const unsigned char* input, int ilen, int& olen);
};
