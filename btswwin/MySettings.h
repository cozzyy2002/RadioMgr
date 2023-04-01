#pragma once

#include "Settings.h"

struct CMySettings : public CSettings::BinaryValue<WINDOWPLACEMENT>::DefaultValueHandler
{
	CMySettings()
		: settings(_T("btswwin"))
		, switchByLcdState(_T("SwitchByLcdState"), TRUE)
		, restoreRadioState(_T("RestoreRadioState"), TRUE)
		, saveWindowPlacement(_T("SaveWindowPlacement"))
		, windowPlacement(_T("WindowPlacement"), this)
	{
		windowPlacement->length = sizeof(WINDOWPLACEMENT);
	}

	using BoolValue = CSettings::Value<BOOL>;
	BoolValue switchByLcdState;
	BoolValue restoreRadioState;
	BoolValue saveWindowPlacement;
	CSettings::BinaryValue<WINDOWPLACEMENT> windowPlacement;

	void load();
	void save();

#pragma region implementation of CSettings::BinaryValue<WINDOWPLACEMENT>::IValueHandler
	virtual bool isChanged(const WINDOWPLACEMENT& a, const WINDOWPLACEMENT& b);
	virtual CString valueToString(const CSettings::BinaryValue<WINDOWPLACEMENT>& value) const override;
#pragma endregion

protected:
	CSettings settings;
};
