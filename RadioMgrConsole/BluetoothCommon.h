#pragma once

#include <Windows.h>
#include <sal.h>
#include <bthsdpdef.h>
#include <bluetoothapis.h>

#include <string>
#include <sstream>

#include "../Common/Assert.h"
#include "../Common/SafeHandle.h"

#define A2ARG(s, x) \
	s.x.rgBytes[5], s.x.rgBytes[4], s.x.rgBytes[3],\
	s.x.rgBytes[2], s.x.rgBytes[1], s.x.rgBytes[0]
#define B2ARG(s, x) s.f##x ? L#x : L"NOT " L#x
#define T2ARG(x) x.wYear, x.wMonth, x.wDay, x.wHour, x.wMinute, x.wSecond

extern std::wstring ClassOfDevice(ULONG value);

extern void HandleDeleteFunc(HANDLE h);
using HRadio = SafeHandle<HANDLE, HandleDeleteFunc>;
using HFile = SafeHandle<HANDLE, HandleDeleteFunc>;
