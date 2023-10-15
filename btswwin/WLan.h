#pragma once

#include <wlanapi.h>
#include <memory>

class CWLan
{
public:
	~CWLan();

	HRESULT start();
	HRESULT stop();

protected:
	struct ClientHandleDeleter {
		using pointer = HANDLE;
		void operator() (HANDLE h);
	};

	std::unique_ptr<HANDLE, ClientHandleDeleter> m_clientHandle;

	static void WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pdata, PVOID pThis);
	HRESULT WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData);
};
