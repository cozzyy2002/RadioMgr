#pragma once

#include <wlanapi.h>
#include <memory>

class CWLan
{
public:
	CWLan();
	~CWLan();

	HRESULT start(HWND hwnd, UINT wndMsg);
	HRESULT stop();

	HRESULT updateInterfaceInfo();
	HRESULT connect();
	HRESULT disconnect();

	struct NotifyParam
	{
		enum class Code
		{
			Ignore,
			Authenticating,
			Authenticated,
			Connecting,
			Connected,
			ConnectFailed,
			Disconnecting,
			Disconnected,
		};

		NotifyParam(Code code, const DOT11_SSID& dot11Ssid, bool isSecurityEnabled)
			: sourceName(_T("Unknown")), codeName(_T("Unknown"))
			, code(code)
			, ssid((char*)dot11Ssid.ucSSID, dot11Ssid.uSSIDLength)
			, isSecurityEnabled(isSecurityEnabled)
		{}

		LPCTSTR sourceName;
		LPCTSTR codeName;
		const Code code;
		const CString ssid;
		const bool isSecurityEnabled;
	};

protected:
	HWND m_hWnd;
	UINT m_wndMsg;

	struct ClientHandleDeleter {
		using pointer = HANDLE;
		void operator() (HANDLE h);
	};

	std::unique_ptr<HANDLE, ClientHandleDeleter> m_clientHandle;

	template<class T>
	struct WlanMemoryDeleter {
		void operator() (T* p) { WlanFreeMemory(p); }
	};

	template<class T>
	using WlanPtr = std::unique_ptr<T, WlanMemoryDeleter<T>>;

	WlanPtr<WLAN_INTERFACE_INFO_LIST> m_interfaceList;

	static void WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pdata, PVOID pThis);
	HRESULT WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData);
};
