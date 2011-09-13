#pragma once

#include "windef.h"

#include "MapViewOfSection.h"

bool SetModuleFilter(ULONG PID, const PUNICODE_STRING filepath, bool filtered);

ULONG GetModuleFilter(ULONG PID, const PUNICODE_STRING filepath);
//fanzhenxing add forͬ�����ٹ���
bool InitBlackCache();
bool AddBlackCache(const PUNICODE_STRING filepath);
bool IsInBlackCache(const PUNICODE_STRING filepath);
bool ClearBlackCache();

ULONG getAllFilter(wchar_t *wcsName, ULONG len );
