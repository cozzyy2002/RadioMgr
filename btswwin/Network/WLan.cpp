#include "pch.h"
#include "WLan.h"

#include "ValueName.h"
#include "../Common/Assert.h"

#include <typeinfo>

#pragma comment(lib, "Wlanapi.lib")

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CWLan")));

void CWLan::ClientHandleDeleter::operator() (HANDLE h) {
    HR_EXPECT_OK(HRESULT_FROM_WIN32(WlanCloseHandle(h, NULL)));
}

CWLan::CWLan()
    : m_hWnd(NULL), m_wndMsg(0)
{
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
    LOG4CXX_INFO_FMT(logger, _T("Opened Wlan Version=%d.%d"),
            WLAN_API_VERSION_MAJOR(dwNegotiatedVersion),
            WLAN_API_VERSION_MINOR(dwNegotiatedVersion)
    );
    m_clientHandle.reset(h);

    // Register WLan notification
    // NOTE:
    //   SOURCE_ACM and SOURCE_MSM are notified.
    //   SOURCE_MSM notifys wlan_notification_msm_connected when Wi-Fi is enabled and *disabled*.
    //   So we register notification only SOURCE_ACM.
    auto source = WLAN_NOTIFICATION_SOURCE_ACM /*| WLAN_NOTIFICATION_SOURCE_MSM*/;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(
        WlanRegisterNotification(h, source, TRUE, WlanNotificationCallback, this, NULL, NULL)
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

using CreateNotificationParam = CWLan::NotifyParam* (*)(PWLAN_NOTIFICATION_DATA pNotificationData);

template<typename T>
CWLan::NotifyParam* createNotificationParam(PWLAN_NOTIFICATION_DATA pNotificationData);
template<typename T>
CWLan::NotifyParam::Code getCode(DWORD notificationCode);

HRESULT CWLan::WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    static const ValueName<DWORD> Sources[] = {
#define ITEM(x, ...) { WLAN_NOTIFICATION_##x, _T(#x), nullptr, __VA_ARGS__ }
        ITEM(SOURCE_NONE),
        ITEM(SOURCE_ACM, createNotificationParam<WLAN_CONNECTION_NOTIFICATION_DATA>),
        ITEM(SOURCE_MSM, createNotificationParam<WLAN_MSM_NOTIFICATION_DATA>),
        ITEM(SOURCE_SECURITY),
        ITEM(SOURCE_IHV),
        ITEM(SOURCE_HNWK),
        ITEM(SOURCE_ONEX),
        ITEM(SOURCE_DEVICE_SERVICE),
#undef ITEM
    };

    static const ValueName<CWLan::NotifyParam::Code> codes[] = {
#define ITEM(x) { CWLan::NotifyParam::Code::x, _T(#x) }
        ITEM(Ignore),
        ITEM(Authenticating),
        ITEM(Authenticated),
        ITEM(Connecting),
        ITEM(Connected),
        ITEM(ConnectFailed),
        ITEM(Disconnecting),
        ITEM(Disconnected),
#undef ITEM
    };

    for(auto& x : Sources) {
        if(x.value & pNotificationData->NotificationSource) {
            LOG4CXX_TRACE(logger,
                x.name
                << _T(", Code = ") << pNotificationData->NotificationCode
                << _T(", Size = ") << pNotificationData->dwDataSize
            );

            auto func = (CreateNotificationParam)x.param;
            if(func) {
                auto notifyParam(func(pNotificationData));
                if(notifyParam) {
                    auto& code = FindValueName(codes, notifyParam->code);
                    LOG4CXX_DEBUG(logger, _T("Wi-Fi ")
                        << x.toString().GetString() << _T(" ")
                        << code.toString().GetString()
                        << _T(": SSID = ") << notifyParam->ssid.GetString()
                        << _T(", ") << (notifyParam->isSecurityEnabled ? _T("Secured") : _T("Unsecured"))
                    );
                    notifyParam->sourceName = x.name;
                    notifyParam->codeName = code.name;
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

template<typename T>
CWLan::NotifyParam* createNotificationParam(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    CWLan::NotifyParam* ret = nullptr;
    auto code = getCode<T>(pNotificationData->NotificationCode);
    if(code != CWLan::NotifyParam::Code::Ignore) {
        if(sizeof(T) <= pNotificationData->dwDataSize) {
            auto pdata = (T*)pNotificationData->pData;
            ret = new CWLan::NotifyParam(code, pdata->dot11Ssid, pdata->bSecurityEnabled);
        } else {
            CA2T typeName(typeid(T).name());
            LOG4CXX_WARN(logger, _T(__FUNCTION__) _T("<") << (LPCTSTR)typeName << _T(">: ")
                << _T("Data size ") << pNotificationData->dwDataSize
                << _T(" is less than expected structure size ") << sizeof(T)
            );
        }
    }
    return ret;
}

template<>
CWLan::NotifyParam::Code getCode<WLAN_CONNECTION_NOTIFICATION_DATA>(DWORD notificationCode)
{
    switch(notificationCode) {
    case wlan_notification_acm_connection_start:
        return CWLan::NotifyParam::Code::Connecting;
    case wlan_notification_acm_connection_complete:
        return CWLan::NotifyParam::Code::Connected;
    case wlan_notification_acm_connection_attempt_fail:
        return CWLan::NotifyParam::Code::ConnectFailed;
    case wlan_notification_acm_disconnecting:
        return CWLan::NotifyParam::Code::Disconnecting;
    case wlan_notification_acm_disconnected:
        return CWLan::NotifyParam::Code::Disconnected;
    default:
        return CWLan::NotifyParam::Code::Ignore;
    }
}

template<>
CWLan::NotifyParam::Code getCode<WLAN_MSM_NOTIFICATION_DATA>(DWORD notificationCode)
{
    switch(notificationCode) {
    case wlan_notification_msm_authenticating:
        return CWLan::NotifyParam::Code::Authenticating;
    case wlan_notification_msm_connected:
        return CWLan::NotifyParam::Code::Connected;
    case wlan_notification_msm_disconnected:
        return CWLan::NotifyParam::Code::Disconnected;
    default:
        return CWLan::NotifyParam::Code::Ignore;
    }
}
