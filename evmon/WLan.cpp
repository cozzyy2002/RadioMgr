#include "WLan.h"

#include "ValueName.h"

#pragma comment(lib, "Wlanapi.lib")

extern void print(LPCTSTR format, ...);

template<class T>
struct WlanMemoryDeleter {
    void operator() (T* p) { WlanFreeMemory(p); }
};

template<class T>
using WlanPtr = std::unique_ptr<T, WlanMemoryDeleter<T>>;


static void WlanNotificationCallback(
    PWLAN_NOTIFICATION_DATA unnamedParam1,
    PVOID unnamedParam2
)
{
    auto pThis = (WLan*)unnamedParam2;
}

WLan::WLan()
{
    HANDLE h;
    DWORD dwNegotiatedVersion;
    if(SUCCEEDED(HR_EXPECT_OK(HRESULT_FROM_WIN32(
        WlanOpenHandle(WLAN_API_VERSION, NULL, &dwNegotiatedVersion, &h))))
    ) {
        print(_T("Opened WLan: Version=%d.%d"),
            WLAN_API_VERSION_MAJOR(dwNegotiatedVersion), WLAN_API_VERSION_MINOR(dwNegotiatedVersion)
        );
        m_clientHandle.reset(h);
        HR_EXPECT_OK(HRESULT_FROM_WIN32(
            WlanRegisterNotification(h, WLAN_NOTIFICATION_SOURCE_ALL, TRUE, WlanNotificationCallback, this, NULL, NULL)
        ));
    }
}

WLan::~WLan()
{
    if(m_clientHandle) {
        HR_EXPECT_OK(HRESULT_FROM_WIN32(
            WlanRegisterNotification(m_clientHandle.get(), WLAN_NOTIFICATION_SOURCE_NONE, TRUE, NULL, NULL, NULL, NULL)
        ));
    }
}

HRESULT WLan::update()
{
    HR_ASSERT(m_clientHandle, E_ILLEGAL_METHOD_CALL);

    WLAN_INTERFACE_INFO_LIST* p;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(WlanEnumInterfaces(m_clientHandle.get(), NULL, &p)));
    WlanPtr<WLAN_INTERFACE_INFO_LIST> wlanInterfaces(p);

    print(_T("Enumerating %d Interfaces"), wlanInterfaces->dwNumberOfItems
    );

    static const ValueName<WLAN_INTERFACE_STATE> WlanInterfaceStates[]{
#define ITEM(x) { wlan_interface_state_##x, _T(#x) }
        ITEM(not_ready),
        ITEM(connected),
        ITEM(ad_hoc_network_formed),
        ITEM(disconnecting),
        ITEM(disconnected),
        ITEM(associating),
        ITEM(discovering),
        ITEM(authenticating)
#undef ITEM
    };

    auto nInterfaces = wlanInterfaces->dwNumberOfItems;
    for(DWORD i = 0; i < nInterfaces; i++) {
        const auto& info = wlanInterfaces->InterfaceInfo[i];
        _tprintf_s(_T("  %s:%s:%s\n"),
            valueToString(info.InterfaceGuid).GetString(),
            info.strInterfaceDescription,
            ValueToString(WlanInterfaceStates, info.isState).GetString()
        );
    }

    return S_OK;
}
