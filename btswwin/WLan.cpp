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

HRESULT CWLan::start()
{
    HANDLE h;
    DWORD dwNegotiatedVersion;
    HR_ASSERT_OK(HRESULT_FROM_WIN32(WlanOpenHandle(WLAN_API_VERSION, NULL, &dwNegotiatedVersion, &h)));
    LOG4CXX_INFO(logger, _T("Opened Version=")
        << WLAN_API_VERSION_MAJOR(dwNegotiatedVersion) << _T(".")
        << WLAN_API_VERSION_MINOR(dwNegotiatedVersion)
    );
    m_clientHandle.reset(h);
    HR_ASSERT_OK(HRESULT_FROM_WIN32(
        WlanRegisterNotification(h, WLAN_NOTIFICATION_SOURCE_ALL, TRUE, WlanNotificationCallback, this, NULL, NULL)
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

void CWLan::WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pdata, PVOID pThis)
{
    ((CWLan*)pThis)->WlanNotificationCallback(pdata);
}

using NotificationFunc = HRESULT(*)(PWLAN_NOTIFICATION_DATA pNotificationData);

HRESULT CWLan::WlanNotificationCallback(PWLAN_NOTIFICATION_DATA pNotificationData)
{
    static const ValueName<DWORD> Sources[] = {
#define ITEM(x, ...) { WLAN_NOTIFICATION_##x, _T(#x), nullptr, __VA_ARGS__ }
        ITEM(SOURCE_NONE),
        ITEM(SOURCE_ACM/*, SourceACM*/),
        ITEM(SOURCE_MSM/*, SourceMCM*/),
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
                << _T(", Code = 0x") << pNotificationData->NotificationCode
                << _T(", Size = ") << pNotificationData->dwDataSize
            );
            auto func = (NotificationFunc)x.param;
            if(func) { func(pNotificationData); }
        }
    }
    return S_OK;
}
