// btsw.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "DeviceRadioState.h"
#include "../Common/Assert.h"

#include <atlbase.h>
#include <string>
#include <memory>

LPCWSTR CLSID_BlueToothRadioManager = L"{afd198ac-5f30-4e89-a789-5ddf60a69366}";

struct Param {
    LPCWSTR str;
    DEVICE_RADIO_STATE newState;
    DEVICE_RADIO_STATE currentState;
};

static const Param params[] = {
    { L"on", DRS_RADIO_ON, DRS_SW_RADIO_OFF },
    { L"off", DRS_SW_RADIO_OFF, DRS_RADIO_ON },
};

int wmain(int argc, wchar_t** argv)
{
    tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg) { _putts(msg); };


    const Param* setParam = nullptr;
    UINT32 timeout = 1;
    if(1 < argc) {
        std::wstring operaton(argv[1]);
        for(auto& param : params) {
            if(operaton.compare(param.str) == 0) {
                setParam = &param;
                break;
            }
        }
    }

    HR_ASSERT_OK(CoInitializeEx(NULL, COINIT_MULTITHREADED));

    // Create IMediaRedioManager instance.
    CComPtr<IMediaRadioManager> radioManager;
    CLSID clsid;
    HR_ASSERT_OK(CLSIDFromString(CLSID_BlueToothRadioManager, &clsid));
    HR_ASSERT_OK(radioManager.CoCreateInstance(clsid));

    // Get collection of IRadioInstance and check if the collection has one or more elements.
    CComPtr<IRadioInstanceCollection> col;
    HR_ASSERT_OK(radioManager->GetRadioInstances(&col));
    UINT32 instanceCount;
    HR_ASSERT_OK(col->GetCount(&instanceCount));
    HR_ASSERT(0 < instanceCount, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

    // Get first IRadioInstance object in the collection and modify it's state as specfied by command line parameter.
    CComPtr<IRadioInstance> instance;
    HR_ASSERT_OK(col->GetAt(0, &instance));
    BSTR friendlyName;
    HR_ASSERT_OK(instance->GetFriendlyName(1033, &friendlyName));
    BSTR signature;
    HR_ASSERT_OK(instance->GetInstanceSignature(&signature));
    DEVICE_RADIO_STATE state;
    HR_ASSERT_OK(instance->GetRadioState(&state));

    std::unique_ptr<DeviceRadioState> currentState(DeviceRadioState::create(state));
    wprintf_s(L"%s:%s State=%s", friendlyName, signature, currentState->str() );

    if(setParam) {
        std::unique_ptr<DeviceRadioState> newState(currentState->createSwitchableState());
        HR_ASSERT(newState, E_ABORT);
        if((setParam->currentState == *currentState) || (setParam->newState == *newState)) {
            HR_ASSERT_OK(instance->SetRadioState(*newState, timeout));
            wprintf_s(L": Complete setting state to %s\n", newState->str());
        } else {
            _putws(L": State is already set");
        }
    } else {
        _putws(L"");
    }

    return 0;
}
