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
	WLan() : m_hwnd(NULL), m_msg(0) {}
	~WLan() { stop(); }

	HRESULT start(HWND, UINT);
	HRESULT stop();
	HRESULT update();

protected:
	std::unique_ptr<HANDLE, ClientHandleDeleter> m_clientHandle;

	// Window handle and message ID to post message of notification callback.
	HWND m_hwnd;
	UINT m_msg;

	static void WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pdata, PVOID pThis);
	HRESULT WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData);
};
