#pragma once

#include "Settings.h"

struct CMySettings : public CSettings::BinaryValue<WINDOWPLACEMENT>::DefaultValueHandler
{
	CMySettings(LPCTSTR companyName, LPCTSTR applicationName);

	using BoolValue = CSettings::Value<BOOL>;
	BoolValue switchByLcdState;
	BoolValue restoreRadioState;
	BoolValue saveWindowPlacement;
	CSettings::Value<int> setRadioStateTimeout;
	CSettings::BinaryValue<WINDOWPLACEMENT> windowPlacement;

	void load();
	void save();

	bool isChanged() const { return settings.isChanged(); }

#pragma region DebugSwitches
	enum {
		// Enable log for connecting device.
		ShowServiceStateGUID = 1,
		// Enable LID open/close debug command in system menu.
		LIDSwitch = 2,
	};

	bool isEnabled(int);

protected:
	CSettings::Value<int> debugSwitches;
#pragma endregion

#pragma region implementation of CSettings::BinaryValue<WINDOWPLACEMENT>::IValueHandler
public:
	virtual bool isChanged(const WINDOWPLACEMENT& a, const WINDOWPLACEMENT& b);
	virtual CString valueToString(const CSettings::BinaryValue<WINDOWPLACEMENT>& value) const override;
#pragma endregion

protected:
	CSettings settings;
};
