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


void WLan::WlanNotificationCallback(
    PWLAN_NOTIFICATION_DATA unnamedParam1,
    PVOID unnamedParam2
)
{
    auto pThis = (WLan*)unnamedParam2;
    pThis->WlanNotificationCallback(unnamedParam1);

    //WIN32_EXPECT(PostMessage(pThis->m_hwnd, pThis->m_msg));
}

using NotificationFunc = HRESULT (*)(PWLAN_NOTIFICATION_DATA pNotificationData);
static HRESULT SourceACM(PWLAN_NOTIFICATION_DATA pNotificationData);
static HRESULT SourceMCM(PWLAN_NOTIFICATION_DATA pNotificationData);

template<> LPCTSTR ValueName<DWORD>::StringFormat = _T("0x%02x");

HRESULT WLan::WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    static const ValueName<DWORD> Sources[] = {
#define ITEM(x, ...) { WLAN_NOTIFICATION_##x, _T(#x), __VA_ARGS__ }
        ITEM(SOURCE_NONE),
        ITEM(SOURCE_ACM, SourceACM),
        ITEM(SOURCE_MSM, SourceMCM),
        ITEM(SOURCE_SECURITY),
        ITEM(SOURCE_IHV),
        ITEM(SOURCE_HNWK),
        ITEM(SOURCE_ONEX),
        ITEM(SOURCE_DEVICE_SERVICE),
#undef ITEM
    };

    for(auto& x : Sources) {
        if(x.value & pNotificationData->NotificationSource) {
            print(_T("WLan: %s, Code=0x%02x, Size=%d"),
                x.toString().GetString(),
                pNotificationData->NotificationCode, pNotificationData->dwDataSize
            );
            auto func = (NotificationFunc)x.param;
            if(func) { func(pNotificationData); }
        }
    }
    return S_OK;
}

template<typename T> HRESULT ConnectionNotify(DWORD size, T* pdata);
using ACMConnectionNotifyFunc = HRESULT (*)(DWORD, WLAN_CONNECTION_NOTIFICATION_DATA*);

/*static*/ HRESULT SourceACM(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    static const ValueName<DWORD> Codes[] = {
#define ITEM(x, ...) { wlan_notification_acm_##x, _T(#x), __VA_ARGS__ }
    ITEM(autoconf_enabled),
    ITEM(autoconf_disabled),
    ITEM(background_scan_enabled),
    ITEM(background_scan_disabled),
    ITEM(bss_type_change),
    ITEM(power_setting_change),
    ITEM(scan_complete),
    ITEM(scan_fail),
    ITEM(connection_start),
    ITEM(connection_complete, ConnectionNotify<WLAN_CONNECTION_NOTIFICATION_DATA>),
    ITEM(connection_attempt_fail),
    ITEM(filter_list_change),
    ITEM(interface_arrival),
    ITEM(interface_removal),
    ITEM(profile_change),
    ITEM(profile_name_change),
    ITEM(profiles_exhausted),
    ITEM(network_not_available),
    ITEM(network_available),
    ITEM(disconnecting),
    ITEM(disconnected, ConnectionNotify<WLAN_CONNECTION_NOTIFICATION_DATA>),
    ITEM(adhoc_network_state_change),
    ITEM(profile_unblocked),
    ITEM(screen_power_change),
    ITEM(profile_blocked),
    ITEM(scan_list_refresh),
    ITEM(operational_state_change),
#undef ITEM
    };
    auto x = FindValueName(Codes, pNotificationData->NotificationCode);
    _tprintf_s(_T("  %s"), x.toString().GetString());
    auto func = (ACMConnectionNotifyFunc)x.param;
    if(func) { func(pNotificationData->dwDataSize, (WLAN_CONNECTION_NOTIFICATION_DATA*)pNotificationData->pData); }
    _tprintf_s(_T("\n"));

    return S_OK;
}

using MSMConnectionNotifyFunc = HRESULT (*)(DWORD, WLAN_MSM_NOTIFICATION_DATA*);

/*static*/ HRESULT SourceMCM(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    static const ValueName<DWORD> Codes[] = {
#define ITEM(x, ...) {  wlan_notification_msm_##x, _T(#x), __VA_ARGS__ }
    ITEM(associating),
    ITEM(associated),
    ITEM(authenticating),
    ITEM(connected, ConnectionNotify<WLAN_MSM_NOTIFICATION_DATA>),
    ITEM(roaming_start),
    ITEM(roaming_end),
    ITEM(radio_state_change),
    ITEM(signal_quality_change),
    ITEM(disassociating),
    ITEM(disconnected, ConnectionNotify<WLAN_MSM_NOTIFICATION_DATA>),
    ITEM(peer_join),
    ITEM(peer_leave),
    ITEM(adapter_removal),
    ITEM(adapter_operation_mode_change),
    ITEM(link_degraded),
    ITEM(link_improved),
#undef ITEM
    };
    auto x = FindValueName(Codes, pNotificationData->NotificationCode);
    _tprintf_s(_T("  %s"), x.toString().GetString());
    auto func = (MSMConnectionNotifyFunc)x.param;
    if(func) { func(pNotificationData->dwDataSize, (WLAN_MSM_NOTIFICATION_DATA*)pNotificationData->pData); }
    _tprintf_s(_T("\n"));

    return S_OK;
}

template<typename T>
HRESULT ConnectionNotify(DWORD size, T* pdata)
{
    std::string ssid((char*)pdata->dot11Ssid.ucSSID, pdata->dot11Ssid.uSSIDLength);
    printf_s(": SSID=`%s`, Security Enabled=%s",
        ssid.c_str(), pdata->bSecurityEnabled ? "true" : "false"
    );
    return S_OK;
}


HRESULT WLan::start(HWND hwnd, UINT msgid)
{
    m_hwnd = hwnd;
    m_msg = msgid;

    HANDLE h;
    DWORD dwNegotiatedVersion;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(WlanOpenHandle(WLAN_API_VERSION, NULL, &dwNegotiatedVersion, &h)));
    _tprintf_s(_T("Opened WLan: Version=%d.%d\n"),
        WLAN_API_VERSION_MAJOR(dwNegotiatedVersion), WLAN_API_VERSION_MINOR(dwNegotiatedVersion)
    );
    m_clientHandle.reset(h);
    HR_ASSERT_OK(HRESULT_FROM_WIN32(
        WlanRegisterNotification(h, WLAN_NOTIFICATION_SOURCE_ALL, TRUE, WlanNotificationCallback, this, NULL, NULL)
    ));

    return S_OK;
}

HRESULT WLan::stop()
{
    if(m_clientHandle) {
        HR_ASSERT_OK(HRESULT_FROM_WIN32(
            WlanRegisterNotification(m_clientHandle.get(), WLAN_NOTIFICATION_SOURCE_NONE, TRUE, NULL, NULL, NULL, NULL)
        ));
        m_clientHandle.reset();
    }
    return S_OK;
}

HRESULT WLan::update()
{
    HR_ASSERT(m_clientHandle, E_ILLEGAL_METHOD_CALL);

    WLAN_INTERFACE_INFO_LIST* p;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(WlanEnumInterfaces(m_clientHandle.get(), NULL, &p)));
    WlanPtr<WLAN_INTERFACE_INFO_LIST> wlanInterfaces(p);

    _tprintf_s(_T("Enumerating %d Interfaces\n"), wlanInterfaces->dwNumberOfItems);

    static const ValueName<WLAN_INTERFACE_STATE> WlanInterfaceStates[] = {
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
