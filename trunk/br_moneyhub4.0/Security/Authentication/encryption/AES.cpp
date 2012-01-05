/**
 *-----------------------------------------------------------*
 *  ��Ȩ���У�  (c), 2010 - 2999, �������ź�ͨ�Ƽ����޹�˾
 *    �ļ�����  AES.cpp
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
#include "AES.h"
#include <string.h>
#include "AESImpl.h"

CAES::CAES(void)
{
	m_key = NULL;
	m_output = NULL;
}

CAES::~CAES(void)
{
	if(m_key)
		delete m_key;

	if(m_output)
		delete m_output;
}

void CAES::SetKey(unsigned char* key, int len)
{
	if(m_key)
		delete m_key;

	m_keylen = len;
	m_key = new unsigned char[len];

	memcpy(m_key, key, len);
}

unsigned char* CAES::Cipher(const unsigned char* input, int ilen, int& olen)
{
	if(m_key == NULL)
		return false;

	if(!(m_keylen == 16 || m_keylen == 24 || m_keylen == 32))
		return false;

	const unsigned char* pInput = input;
	if(m_output)
		delete m_output;

	long textlen = ((ilen + 16 - 1) / 16) * 16;
	olen = textlen;

	unsigned char* extInput = new unsigned char[textlen];
	m_output = new unsigned char[textlen];
	unsigned char* extOutput = m_output;

	memset(extInput, 0, textlen);
	memset(extOutput, 0, textlen);

	memcpy(extInput, input, ilen);
	
	while(textlen > 0)
	{
		CAESImpl AES(m_keylen, m_key);

		AES.Cipher(extInput, extOutput);

		extInput += 16;
		extOutput += 16;

		textlen -= 16;
	}

	return m_output;
}

unsigned char* CAES::DeCipher(const unsigned char* input, int ilen, int& olen)
{
	if(m_key == NULL)
		return false;

	if(!(m_keylen == 16 || m_keylen == 24 || m_keylen == 32))
		return false;

	const unsigned char* pInput = input;
	if(m_output)
		delete m_output;

	long textlen = ((ilen + 16 - 1) / 16) * 16;
	olen = textlen;

	unsigned char* extInput = new unsigned char[textlen];
	m_output = new unsigned char[textlen];
	unsigned char* extOutput = m_output;

	memset(extInput, 0, textlen);
	memset(extOutput, 0, textlen);

	memcpy(extInput, input, ilen);

	while(textlen > 0)
	{
		CAESImpl AES(m_keylen, m_key);

		AES.InvCipher(extInput, extOutput);

		extInput += 16;
		extOutput += 16;

		textlen -= 16;
	}

	return m_output;
}
