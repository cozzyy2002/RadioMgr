// RadioMgrConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <RadioMgr.h>
#include <atlbase.h>

#include "../Common/Assert.h"

using namespace ATL;


static const wchar_t* knownClsIDs[] = {
    // CLSID of IMediaRadioManager               Count Name
    L"{1910E202-236A-43E6-9469-FE0B3149F3D9}",  //  0 Wwan Radio Manager
    L"{3F9FF9AB-AC3E-40BB-BBC9-27B648AD1FB2}",  //  0 Xbox GIP Radio Manager
    L"{833A69FB-5E17-4893-85A5-1EF469217372}",  //  1 Wlan Radio Manager
    L"{afd198ac-5f30-4e89-a789-5ddf60a69366}",  //  1 Bluetooth Radio Media Manager
};

extern HRESULT EnumBluetoothRadios();
extern HRESULT EnumBluetoothDevices();

int wmain(int argc, wchar_t** argv)
{
    tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg) { _putts(msg); };

    _putws(L"---- Enumerating Bluetooth Radios ----");
    HR_ASSERT_OK(EnumBluetoothRadios());

    _putws(L"\n---- Enumerating Bluetooth Devices ----");
    HR_ASSERT_OK(EnumBluetoothDevices());

    return S_OK;

    if(argc < 2) {
        _putws(L"Known CLSIDs of IMediaRadioManager are:");
        for(auto x : knownClsIDs) {
            wprintf_s(L"  %s\n", x);
        }
        return 1;
    }
    auto& strClsid = argv[1];

    HR_ASSERT_OK(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    CComPtr<IMediaRadioManager> radioManager;
    CLSID clsid;
    HR_ASSERT_OK(CLSIDFromString(strClsid, &clsid));
    HR_ASSERT_OK(radioManager.CoCreateInstance(clsid));
    CComPtr<IRadioInstanceCollection> col;
    HR_ASSERT_OK(radioManager->GetRadioInstances(&col));
    UINT32 instanceCount;
    HR_ASSERT_OK(col->GetCount(&instanceCount));

    wprintf_s(L"RadioInstances of CLSID %s: Count=%d\n", strClsid, instanceCount);

    for(UINT32 i = 0; i < instanceCount; i++) {
        CComPtr<IRadioInstance> instance;
        HR_ASSERT_OK(col->GetAt(i, &instance));
        BSTR friendlyName;
        HR_ASSERT_OK(instance->GetFriendlyName(1033, &friendlyName));
        BSTR signature;
        HR_ASSERT_OK(instance->GetInstanceSignature(&signature));
        DEVICE_RADIO_STATE state;
        HR_ASSERT_OK(instance->GetRadioState(&state));

        static const wchar_t* strStateList[] = {
            L"DRS_RADIO_ON",
            L"DRS_SW_RADIO_OFF",
            L"DRS_HW_RADIO_OFF",
            L"DRS_SW_HW_RADIO_OFF",
            L"DRS_HW_RADIO_ON_UNCONTROLLABLE",
            L"DRS_RADIO_INVALID",
            L"DRS_HW_RADIO_OFF_UNCONTROLLABLE",
        };
        const wchar_t* strState = (state < ARRAYSIZE(strStateList) ? strStateList[state] : L"<Unknown state>");
#define STR_BOOL(x) ((x) ? L"true" : L"false")

        wprintf_s(L"%3d %s:%s: RadioState=%s(%d), IsAssociating=%s, IsMultiComm=%s\n", i + 1, friendlyName, signature, strState, state,
            STR_BOOL(instance->IsAssociatingDevice()), STR_BOOL(instance->IsMultiComm())
        );

        //auto newState = state;
        //switch(state) {
        //case DRS_RADIO_ON: newState = DRS_SW_RADIO_OFF; break;
        //case DRS_SW_RADIO_OFF: newState = DRS_RADIO_ON; break;
        //}
        //if(newState != state) {
        //    HR_ASSERT_OK(instance->SetRadioState(newState, 1));
        //    wprintf_s(L"Changed state to %d\n", newState);
        //}
    }
}
