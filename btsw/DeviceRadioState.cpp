#include "DeviceRadioState.h"

struct StateConfig {
	LPCWSTR name;
	DEVICE_RADIO_STATE state;
	DEVICE_RADIO_STATE switchableState;

	static const StateConfig* find(DEVICE_RADIO_STATE state);
};

static const DEVICE_RADIO_STATE InvalidDeviceRadioState = (DEVICE_RADIO_STATE)-1;

#define STATE_CONFIG_ITEM(st, sw) {L#st, st, sw}
static const StateConfig stateConfigList[] = {
	STATE_CONFIG_ITEM(DRS_RADIO_ON, DRS_SW_RADIO_OFF),
	STATE_CONFIG_ITEM(DRS_SW_RADIO_OFF, DRS_RADIO_ON),
	STATE_CONFIG_ITEM(DRS_HW_RADIO_OFF, InvalidDeviceRadioState),
	STATE_CONFIG_ITEM(DRS_SW_HW_RADIO_OFF, InvalidDeviceRadioState),
	STATE_CONFIG_ITEM(DRS_HW_RADIO_ON_UNCONTROLLABLE, InvalidDeviceRadioState),
	STATE_CONFIG_ITEM(DRS_RADIO_INVALID, InvalidDeviceRadioState),
	STATE_CONFIG_ITEM(DRS_HW_RADIO_OFF_UNCONTROLLABLE, InvalidDeviceRadioState),
};

/*static*/ const StateConfig* StateConfig::find(DEVICE_RADIO_STATE state)
{
	for(auto& i : stateConfigList) {
		if(i.state == state) {
			return &i;
		}
	}
	return nullptr;
}

DeviceRadioState* DeviceRadioState::create(DEVICE_RADIO_STATE state)
{
	DeviceRadioState* ret = nullptr;
	auto sc = StateConfig::find(state);
	if(sc) {
		ret = new DeviceRadioState(sc->name, sc->state, sc->switchableState);
	}
	return ret;
}

DeviceRadioState* DeviceRadioState::createSwitchableState() const
{
	DeviceRadioState* ret = nullptr;
	if(m_switchableState != InvalidDeviceRadioState) {
		ret = create(m_switchableState);
	}
	return ret;
}

DeviceRadioState::operator DEVICE_RADIO_STATE() const
{
	return m_state;
}

bool DeviceRadioState::operator==(const DeviceRadioState& that) const
{
	return (this->m_state == that.m_state);
}

LPCWSTR DeviceRadioState::str()
{
	if(m_str.empty()) {
		wchar_t buff[50];
		swprintf_s(buff, L"%s(%d)", m_name, m_state);
		m_str = buff;
	}
	return m_str.c_str();
}

DeviceRadioState::DeviceRadioState(LPCWSTR name, DEVICE_RADIO_STATE state, DEVICE_RADIO_STATE switchableState)
	: m_name(name), m_state(state), m_switchableState(switchableState)
{
}
