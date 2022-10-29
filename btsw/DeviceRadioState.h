#pragma once

#include <RadioMgr.h>
#include <string>

class DeviceRadioState
{
public:
	static DeviceRadioState* create(DEVICE_RADIO_STATE state);
	DeviceRadioState* createSwitchableState() const;

	operator DEVICE_RADIO_STATE() const;
	bool operator ==(const DeviceRadioState&) const;
	LPCWSTR str();

protected:
	DeviceRadioState(LPCWSTR name, DEVICE_RADIO_STATE state, DEVICE_RADIO_STATE switchableState);
	DeviceRadioState() = delete;
	DeviceRadioState(const DeviceRadioState&) = delete;

	std::wstring m_str;
	LPCWSTR m_name;
	DEVICE_RADIO_STATE m_state;
	DEVICE_RADIO_STATE m_switchableState;
};
