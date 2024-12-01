#pragma once

#include "Settings.h"

class CMySettings : public CSettings
{
public:
	CMySettings(LPCTSTR companyName, LPCTSTR applicationName);

	CRegistryKey regKeyRoot;
	CRegistryKey regKeyBluetooth;
	CRegistryKey regKeyNetwork;

	Value<bool> switchByLcdState;
	Value<bool> restoreRadioState;
	Value<int> setRadioOnDelay;
	Value<int> setRadioStateTimeout;
	Value<bool> autoCheckRadioInstance;
	Value<bool> autoSelectDevice;
	Value<int> bluetoothPollingTimer;
	Value<bool> saveWindowPlacement;
	BinaryValue<WINDOWPLACEMENT> windowPlacement;

	// Specifies condition to connect VPN.
	enum class VpnConnection
	{
		None,				// Never connect.
		UnsecuredWiFi,		// When unsecured Wi-Fi is connected.
		WiFi,				// When Wi-Fi is connected.
		Any,				// When any network connection is available.
	};
	EnumValue<VpnConnection> vpnConnection;
	Value<CString> vpnName;
	Value<int> vpnConnectionDelay;
	Value<int> vpnConnectionRetry;

	// Specifies triggers to perform an action.
	enum class Trigger
	{
		None			= 0,
		LidOpen			= 1 << 0,
		LidClose		= 1 << 1,
		LidOpenClose	= LidOpen | LidClose,
		PowerSourceAc	= 1 << 2,
		PowerSourceDc	= 1 << 3,
		PowerSourceHot	= 1 << 4,
		PowerSourceChanged = PowerSourceAc | PowerSourceDc | PowerSourceHot,
		ConsoleDisplayOff		= 1 << 5,
		ConsoleDisplayOn		= 1 << 6,
		ConsoleDisplayDimmed	= 1 << 7,
		ConsoleDisplayChanged = ConsoleDisplayOff | ConsoleDisplayOn | ConsoleDisplayDimmed,
		BatteryLevelLow			= 1 << 8,
		BatteryLevelHigh		= 1 << 9,
		BatteryLevelFull		= 1 << 10,
		BatteryLevelChanged		= BatteryLevelLow | BatteryLevelHigh | BatteryLevelFull,
	};

	EnumValue<Trigger> batteryLogTrigger;

	Value<int> batteryLogLow;
	Value<int> batteryLogHigh;

	void load();
	void save();

	CString getRegistryKeyName(bool isRelative = false) const;

#pragma region DebugSwitches
	enum class DebugSwitch
	{
		None				= 0,
		LogServiceStateGUID	= 1 << 0,	// Enable log for connecting device.
		LIDSwitch			= 1 << 1,	// Enable LID open/close debug command in system menu.
	};

	bool isEnabled(DebugSwitch);

protected:
	EnumValue<DebugSwitch> debugSwitches;
#pragma endregion

	// Implementation of BinaryValue<WINDOWPLACEMENT>::IValueHandler
	struct WindowPlacementValueHandler : public BinaryValue<WINDOWPLACEMENT>::DefaultValueHandler
	{
		explicit WindowPlacementValueHandler(CMySettings* owner) : m_owner(owner) {}

		virtual bool isChanged(const WINDOWPLACEMENT& a, const WINDOWPLACEMENT& b) override;
		virtual CString valueToString(const BinaryValue<WINDOWPLACEMENT>& value) const override;

	protected:
		CMySettings* m_owner;
	};
	WindowPlacementValueHandler m_windowPlacementValueHandler;
};
