// btsw.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <RadioMgr.h>
#include <atlbase.h>
#include <string>

#include "../Common/Assert.h"

LPCWSTR CLSID_BlueToothRadioManager = L"{afd198ac-5f30-4e89-a789-5ddf60a69366}";

struct State {
    LPCWSTR name;
    DEVICE_RADIO_STATE value;

    static const State* find(DEVICE_RADIO_STATE state);
    std::wstring toString() const;
};

#define STATE_ITEM(x) { L#x, x }
static const State AllStates[] = {
    STATE_ITEM(DRS_RADIO_ON),
    STATE_ITEM(DRS_SW_RADIO_OFF),
    STATE_ITEM(DRS_HW_RADIO_OFF),
    STATE_ITEM(DRS_SW_HW_RADIO_OFF),
    STATE_ITEM(DRS_HW_RADIO_ON_UNCONTROLLABLE),
    STATE_ITEM(DRS_RADIO_INVALID),
    STATE_ITEM(DRS_HW_RADIO_OFF_UNCONTROLLABLE),
};

static auto& StateOn = AllStates[0];
static auto& StateOff = AllStates[1];

/*static*/ const State* State::find(DEVICE_RADIO_STATE state)
{
    for(auto& s : AllStates) {
        if(s.value == state) { return &s; }
    }

    static const State UnknownState = {L"Unknwn State", (DEVICE_RADIO_STATE)-1};
    return &UnknownState;
}

std::wstring State::toString() const
{
    wchar_t str[60];
    swprintf_s(str, L"%s(%d)", name, value);
    return str;
}

struct Param {
    LPCWSTR str;
    const State& newState;
    const State& currentState;
};

static const Param params[] = {
    { L"on", StateOn, StateOff },
    { L"off", StateOff, StateOn },
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

    wprintf_s(L"%s:%s State=%s", friendlyName, signature, State::find(state)->toString().c_str() );

    if(setParam) {
        if(state != setParam->newState.value) {
            HR_ASSERT_OK(instance->SetRadioState(setParam->newState.value, timeout));
            wprintf_s(L": Complete setting state to %s\n", setParam->newState.toString().c_str());
        } else {
            _putws(L": State is already set");
        }
    } else {
        _putws(L"");
    }

    return 0;
}
