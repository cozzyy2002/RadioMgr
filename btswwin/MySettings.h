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

#pragma region implementation of CSettings::BinaryValue<WINDOWPLACEMENT>::IValueHandler
	virtual bool isChanged(const WINDOWPLACEMENT& a, const WINDOWPLACEMENT& b);
	virtual CString valueToString(const CSettings::BinaryValue<WINDOWPLACEMENT>& value) const override;
#pragma endregion

protected:
	CSettings settings;
};
