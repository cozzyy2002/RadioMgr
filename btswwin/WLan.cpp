#include "pch.h"
#include "WLan.h"

#include "ValueName.h"
#include "../Common/Assert.h"

#pragma comment(lib, "Wlanapi.lib")

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CWLan")));

void CWLan::ClientHandleDeleter::operator() (HANDLE h) {
    HR_EXPECT_OK(HRESULT_FROM_WIN32(WlanCloseHandle(h, NULL)));
}

CWLan::~CWLan()
{
    stop();
}

HRESULT CWLan::start(HWND hwnd, UINT wndMsg)
{
    m_hWnd = hwnd;
    m_wndMsg = wndMsg;

    HANDLE h;
    DWORD dwNegotiatedVersion;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(WlanOpenHandle(WLAN_API_VERSION, NULL, &dwNegotiatedVersion, &h)));
    LOG4CXX_INFO(logger, _T("Opened Version=")
        << WLAN_API_VERSION_MAJOR(dwNegotiatedVersion) << _T(".")
        << WLAN_API_VERSION_MINOR(dwNegotiatedVersion)
    );
    m_clientHandle.reset(h);

    // Register WLan notification
    // NOTE:
    //   SOURCE_ACM and SOURCE_MSM are notified.
    //   SOURCE_MSM notifys wlan_notification_msm_connected when Wi-Fi is enabled and *disabled*.
    //   So we register notification only SOURCE_ACM.
    HR_ASSERT_OK(HRESULT_FROM_WIN32(
        WlanRegisterNotification(h, WLAN_NOTIFICATION_SOURCE_ACM, TRUE, WlanNotificationCallback, this, NULL, NULL)
    ));

    return S_OK;
}

HRESULT CWLan::stop()
{
    if(m_clientHandle) {
        HR_ASSERT_OK(HRESULT_FROM_WIN32(
            WlanRegisterNotification(m_clientHandle.get(), WLAN_NOTIFICATION_SOURCE_NONE, TRUE, NULL, NULL, NULL, NULL)
        ));
        m_clientHandle.reset();
    }
    return S_OK;
}

void CWLan::WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData, PVOID pThis)
{
    ((CWLan*)pThis)->WlanNotificationCallback(pNotificationData);
}

using CheckNotificationCode = CWLan::NotifyParam* (*)(PWLAN_NOTIFICATION_DATA pNotificationData, DWORD notificationCode);

template<typename T, DWORD Connected, DWORD Disconnected>
CWLan::NotifyParam* checkNotificationCode(PWLAN_NOTIFICATION_DATA pNotificationData, DWORD notificationCode)
{
    bool isConnected;
    switch(pNotificationData->NotificationCode) {
    case Connected:
        isConnected = true;
        break;
    case Disconnected:
        isConnected = false;
        break;
    default:
        return nullptr;
    }
    auto pdata = (T*)pNotificationData->pData;
    return new CWLan::NotifyParam(isConnected, pdata->dot11Ssid, pdata->bSecurityEnabled);
}

HRESULT CWLan::WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    static const ValueName<DWORD> Sources[] = {
#define ITEM(x, ...) { WLAN_NOTIFICATION_##x, _T(#x), nullptr, __VA_ARGS__ }
        ITEM(SOURCE_NONE),
        ITEM(SOURCE_ACM, checkNotificationCode<WLAN_CONNECTION_NOTIFICATION_DATA,
            wlan_notification_acm_connection_complete, wlan_notification_acm_disconnected>),
        ITEM(SOURCE_MSM, checkNotificationCode<WLAN_MSM_NOTIFICATION_DATA,
            wlan_notification_msm_connected, wlan_notification_msm_disconnected>),
        ITEM(SOURCE_SECURITY),
        ITEM(SOURCE_IHV),
        ITEM(SOURCE_HNWK),
        ITEM(SOURCE_ONEX),
        ITEM(SOURCE_DEVICE_SERVICE),
#undef ITEM
    };

    for(auto& x : Sources) {
        if(x.value & pNotificationData->NotificationSource) {
            LOG4CXX_DEBUG(logger,
                x.toString().GetString()
                << _T(", Code = ") << std::hex << pNotificationData->NotificationCode
                << _T(", Size = ") << std::dec << pNotificationData->dwDataSize
            );

            auto func = (CheckNotificationCode)x.param;
            if(func) {
                auto notifyParam(func(pNotificationData, pNotificationData->NotificationCode));
                if(notifyParam) {
                    LOG4CXX_DEBUG(logger,
                        (notifyParam->isConnected ? _T("Connected") : _T("Disconnected"))
                        << _T(" SSID = '") << notifyParam->ssid.GetString()
                        << _T("`, SecurityEnabled = ") << (notifyParam->isSecurityEnabled ? _T("true") : _T("false"))
                    );

                    if(FAILED(WIN32_EXPECT(
                        PostMessage(m_hWnd, m_wndMsg, 0, (LPARAM)notifyParam)
                    ))) {
                        delete notifyParam;
                    }
                }
            }
        }
    }
    return S_OK;
}
