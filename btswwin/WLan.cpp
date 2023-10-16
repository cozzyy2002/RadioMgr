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

using CheckConnected = CWLan::ConnectedParam* (*)(PWLAN_NOTIFICATION_DATA pNotificationData, DWORD notificationCode);

template<typename T, DWORD Connected>
CWLan::ConnectedParam* checkConnected(PWLAN_NOTIFICATION_DATA pNotificationData, DWORD notificationCode)
{
    if(pNotificationData->NotificationCode == Connected) {
        auto pdata = (T*)pNotificationData->pData;
        return new CWLan::ConnectedParam(pdata->dot11Ssid, pdata->bSecurityEnabled);
    } else {
        return nullptr;
    }
}

HRESULT CWLan::WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    static const ValueName<DWORD> Sources[] = {
#define ITEM(x, ...) { WLAN_NOTIFICATION_##x, _T(#x), nullptr, __VA_ARGS__ }
        ITEM(SOURCE_NONE),
        ITEM(SOURCE_ACM, checkConnected<WLAN_CONNECTION_NOTIFICATION_DATA, wlan_notification_acm_connection_complete>),
        ITEM(SOURCE_MSM, checkConnected<WLAN_MSM_NOTIFICATION_DATA, wlan_notification_msm_connected>),
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

            auto func = (CheckConnected)x.param;
            if(func) {
                auto connectedParam(func(pNotificationData, pNotificationData->NotificationCode));
                if(connectedParam) {
                    LOG4CXX_INFO(logger,
                        _T("Connected SSID = '") << connectedParam->ssid.GetString() 
                        << _T("`, SecurityEnabled = ") << (connectedParam->isSecurityEnabled ? _T("true") : _T("false"))
                    );

                    if(FAILED(WIN32_EXPECT(
                        PostMessage(m_hWnd, m_wndMsg, 0, (LPARAM)connectedParam)
                    ))) {
                        delete connectedParam;
                    }
                }
            }
        }
    }
    return S_OK;
}
