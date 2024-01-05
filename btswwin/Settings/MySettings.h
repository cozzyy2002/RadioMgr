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

	void load();

	CString getRegistryKeyName(bool isRelative = false) const;

#pragma region DebugSwitches
	enum class DebugSwitch
	{
		// Enable log for connecting device.
		LogServiceStateGUID = 1,
		// Enable LID open/close debug command in system menu.
		LIDSwitch = 2,
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
