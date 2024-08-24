#include "pch.h"
#include "MySettings.h"
#include "ValueName.h"

static const ValueName<HKEY> RootKey = VALUE_NAME_ITEM(HKEY_CURRENT_USER);

// CMySettings members

CMySettings::CMySettings(LPCTSTR companyName, LPCTSTR applicationName)
	: switchByLcdState(regKeyBluetooth, _T("SwitchByLcdState"), true)
	, restoreRadioState(regKeyBluetooth, _T("RestoreRadioState"), true)
	, setRadioOnDelay(regKeyBluetooth, _T("SetRadioOnDelay"), 4)
	, setRadioStateTimeout(regKeyBluetooth, _T("SetRadioStateTimeout"), 1)
	, autoCheckRadioInstance(regKeyBluetooth, _T("AutoCheckRadioInstance"), true)
	, autoSelectDevice(regKeyBluetooth, _T("AudoSelectDevice"))
	, bluetoothPollingTimer(regKeyBluetooth, _T("BluetoothPollingTimer"), 1)
	, saveWindowPlacement(regKeyRoot, _T("SaveWindowPlacement"))
	, windowPlacement(regKeyRoot, _T("WindowPlacement"), &m_windowPlacementValueHandler)
	, vpnConnection(regKeyNetwork, _T("VpnConnection"), VpnConnection::None)
	, vpnName(regKeyNetwork, _T("VpnName"))
	, vpnConnectionDelay(regKeyNetwork, _T("VpnConnectionDelay"), 2)
	, vpnConnectionRetry(regKeyNetwork, _T("VpnConnectionRetry"))
	, batteryLogTrigger(regKeyRoot, _T("BatteryLogTrigger"), Trigger::LidOpenClose)
	, debugSwitches(regKeyRoot, _T("DebugSwitches"), DebugSwitch::None)
	, m_windowPlacementValueHandler(this)
{
	static const auto subKeyFormat = _T("Software\\%s\\%s")
#ifdef _DEBUG
		_T(".debug");
#endif
	;
	
	CString subKey;
	subKey.Format(subKeyFormat, companyName, applicationName);
	regKeyRoot.attach(RootKey.value, subKey);
	regKeyBluetooth.attach(regKeyRoot, _T("Bluetooth"));
	regKeyNetwork.attach(regKeyRoot, _T("Network"));
}

void CMySettings::load()
{
	regKeyRoot.open();

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
		&vpnConnectionDelay, &vpnConnectionRetry,
		&batteryLogTrigger,
		&debugSwitches
	};
	HR_EXPECT_OK(CSettings::load(valueList));

	regKeyRoot.close();
}

void CMySettings::save()
{
	regKeyRoot.open();
	HR_EXPECT_OK(CSettings::save());
	regKeyRoot.close();
}

CString CMySettings::getRegistryKeyName(bool isRelative /*= false*/) const
{
	CString ret;
	ret.Format(_T("%s\\%s"),
		RootKey.name, regKeyRoot.getFullKeyName(isRelative).GetString()
	);
	return ret;
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
	return (flag & debugSwitches);
}
