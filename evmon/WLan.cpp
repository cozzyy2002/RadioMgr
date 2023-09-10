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


HRESULT WLan::update()
{
    HANDLE h;
    DWORD dwNegotiatedVersion;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(WlanOpenHandle(WLAN_API_VERSION, NULL, &dwNegotiatedVersion, &h)));
    m_clientHandle.reset(h);

    WLAN_INTERFACE_INFO_LIST* p;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(WlanEnumInterfaces(h, NULL, &p)));
    WlanPtr<WLAN_INTERFACE_INFO_LIST> wlanInterfaces(p);

    print(_T("Opened WLan: Version=%d.%d, %d Interfaces"),
        WLAN_API_VERSION_MAJOR(dwNegotiatedVersion), WLAN_API_VERSION_MINOR(dwNegotiatedVersion),
        wlanInterfaces->dwNumberOfItems
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
