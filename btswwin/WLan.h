#pragma once

#include <wlanapi.h>
#include <memory>

class CWLan
{
public:
	~CWLan();

	HRESULT start();
	HRESULT stop();

	struct ConnectedParam
	{
		ConnectedParam(const DOT11_SSID& dot11Ssid, bool isSecurityEnabled)
			: ssid((char*)dot11Ssid.ucSSID, dot11Ssid.uSSIDLength)
			, isSecurityEnabled(isSecurityEnabled)
		{}

		const CString ssid;
		const bool isSecurityEnabled;
	};

protected:
	struct ClientHandleDeleter {
		using pointer = HANDLE;
		void operator() (HANDLE h);
	};

	std::unique_ptr<HANDLE, ClientHandleDeleter> m_clientHandle;

	static void WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pdata, PVOID pThis);
	HRESULT WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData);
};
