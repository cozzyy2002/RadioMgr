// btsw.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "DeviceRadioState.h"
#include "../Common/Assert.h"

#include <atlbase.h>
#include <string>
#include <memory>

LPCWSTR CLSID_BlueToothRadioManager = L"{afd198ac-5f30-4e89-a789-5ddf60a69366}";


using OperationFunc = bool (*)(DeviceRadioState*, DeviceRadioState*);

struct OperationConfig {
    LPCWSTR str;
    OperationFunc func;
};

static bool operationOn(DeviceRadioState* currentState, DeviceRadioState* newState);
static bool operationOff(DeviceRadioState* currentState, DeviceRadioState* newState);
static bool operationToggle(DeviceRadioState* currentState, DeviceRadioState* newState);

static const OperationConfig operations[] = {
    { L"on", operationOn },
    { L"off", operationOff },
    { L"toggle", operationToggle },
};

int wmain(int argc, wchar_t** argv)
{
    tsm::Assert::onAssertFailedWriter = [](LPCTSTR msg) { _putts(msg); };


    OperationFunc operationFunc = nullptr;
    UINT32 timeout = 1;
    if(1 < argc) {
        auto operatonStr(argv[1]);
        for(auto& operation : operations) {
            if(_wcsicmp(operatonStr, operation.str) == 0) {
                operationFunc = operation.func;
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
#define STR_BOOL(x) ((x) ? L"true" : L"false")
    wprintf_s(L"%s:%s: State=%s, IsAssociatingDevice=%s, IsMultiComm=%s\n",
        friendlyName, signature, currentState->name(),
        STR_BOOL(instance->IsAssociatingDevice()), STR_BOOL(instance->IsMultiComm()));

    if(operationFunc) {
        std::unique_ptr<DeviceRadioState> newState(currentState->createSwitchableState());
        HR_ASSERT(newState, E_ABORT);
        if(operationFunc(currentState.get(), newState.get())) {
            // Change state.
            HR_ASSERT_OK(instance->SetRadioState(*newState, timeout));
            wprintf_s(L"Complete setting state to %s\n", newState->name());
        } else {
            _putws(L"State is already set");
        }
    }

    return 0;
}

bool operationOn(DeviceRadioState* currentState, DeviceRadioState* newState)
{
    return (*currentState == DRS_SW_RADIO_OFF);
}

bool operationOff(DeviceRadioState* currentState, DeviceRadioState* newState)
{
    return (*currentState == DRS_RADIO_ON);
}

bool operationToggle(DeviceRadioState* currentState, DeviceRadioState* newState)
{
    // newState may not nullptr.
    // So always state can be toggled.
    return true;
}
