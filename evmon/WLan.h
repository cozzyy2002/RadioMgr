#pragma once

#include <Windows.h>
// https://learn.microsoft.com/en-us/windows/win32/api/wlanapi/
#include <wlanapi.h>
#include <atlbase.h>
#include <memory>

#include "../Common/Assert.h"

struct ClientHandleDeleter {
	using pointer = HANDLE;
	void operator() (HANDLE h) {
		HR_EXPECT_OK(HRESULT_FROM_WIN32(WlanCloseHandle(h, NULL)));
	}
};

class WLan
{
public:
	WLan() {}

	HRESULT update();

protected:
	std::unique_ptr<HANDLE, ClientHandleDeleter> m_clientHandle;
};
