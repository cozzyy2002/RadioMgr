#pragma once

#include "Settings.h"

struct CMySettings : public CSettings::BinaryValue<WINDOWPLACEMENT>::DefaultValueHandler
{
	CMySettings(LPCTSTR companyName, LPCTSTR applicationName);

	CSettings::Value<bool> switchByLcdState;
	CSettings::Value<bool> restoreRadioState;
	CSettings::Value<int> setRadioOnDelay;
	CSettings::Value<int> setRadioStateTimeout;
	CSettings::Value<bool> autoCheckRadioInstance;
	CSettings::Value<bool> autoSelectDevice;
	CSettings::Value<bool> saveWindowPlacement;
	CSettings::BinaryValue<WINDOWPLACEMENT> windowPlacement;

	void load();
	void save();

	bool isChanged() const { return settings.isChanged(); }

#pragma region DebugSwitches
	enum class DebugSwitch{
		// Enable log for connecting device.
		LogServiceStateGUID = 1,
		// Enable LID open/close debug command in system menu.
		LIDSwitch = 2,
	};

	bool isEnabled(DebugSwitch);

protected:
	CSettings::EnumValue<DebugSwitch> debugSwitches;
#pragma endregion

#pragma region implementation of CSettings::BinaryValue<WINDOWPLACEMENT>::IValueHandler
public:
	virtual bool isChanged(const WINDOWPLACEMENT& a, const WINDOWPLACEMENT& b) override;
	virtual CString valueToString(const CSettings::BinaryValue<WINDOWPLACEMENT>& value) const override;
#pragma endregion

protected:
	CSettings settings;
};
