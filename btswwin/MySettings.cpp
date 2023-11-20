#include "pch.h"
#include "MySettings.h"
#include "ValueName.h"

// CMySettings members

CMySettings::CMySettings(LPCTSTR companyName, LPCTSTR applicationName)
	: CSettings(companyName, applicationName)
	, switchByLcdState(_T("SwitchByLcdState"), true)
	, restoreRadioState(_T("RestoreRadioState"), true)
	, setRadioOnDelay(_T("SetRadioOnDelay"), 4)
	, setRadioStateTimeout(_T("SetRadioStateTimeout"), 1)
	, autoCheckRadioInstance(_T("AutoCheckRadioInstance"), true)
	, autoSelectDevice(_T("AudoSelectDevice"))
	, bluetoothPollingTimer(_T("BluetoothPollingTimer"), 1)
	, saveWindowPlacement(_T("SaveWindowPlacement"))
	, windowPlacement(_T("WindowPlacement"), &m_windowPlacementValueHandler)
	, vpnConnection(_T("VpnConnection"), VpnConnection::None)
	, vpnName(_T("VpnName"))
	, vpnConnectionDelay(_T("VpnConnectionDelay"), 2)
	, vpnConnectionRetry(_T("VpnConnectionRetry"))
	, debugSwitches(_T("DebugSwitches"))
	, m_windowPlacementValueHandler(this)
{
}

void CMySettings::load()
{
	IValue* valueList[] = {
		&switchByLcdState,
		&restoreRadioState,
		&setRadioOnDelay,
		&setRadioStateTimeout,
		&autoCheckRadioInstance,
		&autoSelectDevice,
		&bluetoothPollingTimer,
		&saveWindowPlacement,
		&windowPlacement,
		&vpnConnection, &vpnName,
		&vpnConnectionDelay, &vpnConnectionRetry
	};
	HR_EXPECT_OK(CSettings::load(valueList));
}

bool CMySettings::WindowPlacementValueHandler::isChanged(const WINDOWPLACEMENT& a, const WINDOWPLACEMENT& b)
{
	// If saving window position is not necessary, return as unchnged.
	if(!m_owner->saveWindowPlacement) { return false; }

	return
		(a.showCmd != b.showCmd)
		|| (a.rcNormalPosition.left != b.rcNormalPosition.left)
		|| (a.rcNormalPosition.top != b.rcNormalPosition.top)
		|| (a.rcNormalPosition.right != b.rcNormalPosition.right)
		|| (a.rcNormalPosition.bottom != b.rcNormalPosition.bottom)
		;
}

CString CMySettings::WindowPlacementValueHandler::valueToString(const BinaryValue<WINDOWPLACEMENT>& value) const
{
	static const ValueName<UINT> ShowCmds[] = {
		VALUE_NAME_ITEM(SW_HIDE),
		VALUE_NAME_ITEM(SW_NORMAL),
		VALUE_NAME_ITEM(SW_SHOWMINIMIZED),
		VALUE_NAME_ITEM(SW_SHOWMAXIMIZED),
		VALUE_NAME_ITEM(SW_SHOWNOACTIVATE),
		VALUE_NAME_ITEM(SW_SHOW),
		VALUE_NAME_ITEM(SW_MINIMIZE),
		VALUE_NAME_ITEM(SW_SHOWMINNOACTIVE),
		VALUE_NAME_ITEM(SW_SHOWNA),
		VALUE_NAME_ITEM(SW_RESTORE),
		VALUE_NAME_ITEM(SW_SHOWDEFAULT),
		VALUE_NAME_ITEM(SW_FORCEMINIMIZE),
	};

	auto& wp = (const WINDOWPLACEMENT&)value;
	auto& rc = wp.rcNormalPosition;
	CString str;
	str.Format(_T("Position=%d:%d, Size=%dx%d, showCmd=%s"),
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		ValueToString(ShowCmds, wp.showCmd).GetString());
	return str;
}

template<>
const DWORD CSettings::BinaryValue<WINDOWPLACEMENT>::RegType = REG_BINARY;

bool CMySettings::isEnabled(DebugSwitch flag)
{
	debugSwitches.read(this);
	return (flag & debugSwitches);
}
