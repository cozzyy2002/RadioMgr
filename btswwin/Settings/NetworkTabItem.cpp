// NetworkTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "NetworkTabItem.h"
#include "MySettings.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CNetworkTabItem")));

// CNetworkTabItem dialog

IMPLEMENT_DYNAMIC(CNetworkTabItem, CDialogEx)

CNetworkTabItem::CNetworkTabItem(CMySettings& settings)
	: CTabItem(IDD_SETTINGS_NETWORK, _T("Network"), settings)
{
	addController(m_settings.vpnName, m_vpnName);
	addController(new VpnConnectionController(this, m_settings));
	addController(m_settings.vpnConnectionDelay, m_vpnConnectionDelay);
	addController(m_settings.vpnConnectionRetry, m_vpnConnectionRetry);
}

CNetworkTabItem::~CNetworkTabItem()
{
}

bool CNetworkTabItem::isValid() const
{
	// If [Connect VPN] is checked, VPN name should not be empty.
	CString vpnName;
	m_vpnName.GetWindowText(vpnName);
	if(m_connectVpn.GetCheck() && vpnName.IsEmpty()) {
		return false;
	}

	return true;
}

void CNetworkTabItem::updateUIState()
{
	// Set enabled state of the controls depending on whether [Connect VPN] is checked or not.
	static const int ids[] = {
		IDC_EDIT_VPN_NAME,
		IDC_RADIO_VPN_CONNECTION, IDC_RADIO_VPN_CONNECTION_WIFI, IDC_RADIO_VPN_CONNECTION_ANY,
		IDC_STATIC_VPN_CONNECTION_DELAY, IDC_EDIT_VPN_CONNECTION_DELAY, IDC_SPIN_VPN_CONNECTION_DELAY,
		IDC_STATIC_VPN_CONNECTION_RETRY, IDC_EDIT_VPN_CONNECTION_RETRY, IDC_SPIN_VPN_CONNECTION_RETRY,
	};
	auto enable = isButtonChecked(m_connectVpn);
	for(auto id : ids) {
		GetDlgItem(id)->EnableWindow(enable);
	}
}

void CNetworkTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_CONNECT_VPN, m_connectVpn);
	DDX_Control(pDX, IDC_EDIT_VPN_NAME, m_vpnName);
	DDX_Control(pDX, IDC_EDIT_VPN_CONNECTION_DELAY, m_vpnConnectionDelay);
	DDX_Control(pDX, IDC_SPIN_VPN_CONNECTION_DELAY, m_vpnConnectionDelaySpin);
	DDX_Control(pDX, IDC_EDIT_VPN_CONNECTION_RETRY, m_vpnConnectionRetry);
	DDX_Control(pDX, IDC_SPIN_VPN_CONNECTION_RETRY, m_vpnConnectionRetrySpin);
}

BOOL CNetworkTabItem::OnInitDialog()
{
	CTabItem::OnInitDialog();

	m_vpnConnectionDelaySpin.SetRange(0, 60);
	m_vpnConnectionRetrySpin.SetRange(0, 10);

	updateUIState();

	return 0;
}


BEGIN_MESSAGE_MAP(CNetworkTabItem, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_CONNECT_VPN, &CNetworkTabItem::OnClickedCheckConnectVpn)
	ON_BN_CLICKED(IDC_RADIO_VPN_CONNECTION, &CNetworkTabItem::OnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_VPN_CONNECTION_WIFI, &CNetworkTabItem::OnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_VPN_CONNECTION_ANY, &CNetworkTabItem::OnClickedRadio)
	ON_EN_CHANGE(IDC_EDIT_VPN_NAME, &CNetworkTabItem::OnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_VPN_CONNECTION_DELAY, &CNetworkTabItem::OnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_VPN_CONNECTION_RETRY, &CNetworkTabItem::OnChangeEdit)
END_MESSAGE_MAP()


// CNetworkTabItem message handlers


void CNetworkTabItem::OnClickedCheckConnectVpn()
{
	updateUIState();
	notifyValueChanged();
}


void CNetworkTabItem::OnChangeEdit()
{
	notifyValueChanged();
}


void CNetworkTabItem::OnClickedRadio()
{
	notifyValueChanged();
}

#pragma region Implementation of VpnConnectionController
void VpnConnectionController::setValueToCtrl()
{
	auto isConnectVpnChecked = true;
	auto buttonIdToCheck = 0;
	switch((CMySettings::VpnConnection)m_value) {
	case CMySettings::VpnConnection::None:
		buttonIdToCheck = IDC_RADIO_VPN_CONNECTION;
		isConnectVpnChecked = false;
		break;
	case CMySettings::VpnConnection::UnsecuredWiFi:
		buttonIdToCheck = IDC_RADIO_VPN_CONNECTION;
		break;
	case CMySettings::VpnConnection::WiFi:
		buttonIdToCheck = IDC_RADIO_VPN_CONNECTION_WIFI;
		break;
	case CMySettings::VpnConnection::Any:
		buttonIdToCheck = IDC_RADIO_VPN_CONNECTION_ANY;
		break;
	default:
		LOG4CXX_ERROR_FMT(logger, _T("Unknown VpnConnection value: %d"), (int)m_value);
		return;
	}

	setButtonCheck(m_dlg->m_connectVpn, isConnectVpnChecked);
	m_dlg->CheckRadioButton(IDC_RADIO_VPN_CONNECTION, IDC_RADIO_VPN_CONNECTION_ANY, buttonIdToCheck);
}

void VpnConnectionController::getValueFromCtrl()
{
	m_value = getVpnConnection();
}

bool VpnConnectionController::isChanged() const
{
	auto eValue = getVpnConnection();
	return (m_value != eValue) || m_value.isChanged();
}
#pragma endregion

// Returns CMySettings::VpnConnection value
// specified by [Connect VPN] check box and radio buttons.
CMySettings::VpnConnection VpnConnectionController::getVpnConnection() const
{
	auto ret = CMySettings::VpnConnection::None;
	if(m_dlg->m_connectVpn.GetCheck()) {
		auto checkedButtonId = m_dlg->GetCheckedRadioButton(IDC_RADIO_VPN_CONNECTION, IDC_RADIO_VPN_CONNECTION_ANY);
		switch(checkedButtonId) {
		case IDC_RADIO_VPN_CONNECTION:
			ret = CMySettings::VpnConnection::UnsecuredWiFi;
			break;
		case IDC_RADIO_VPN_CONNECTION_WIFI:
			ret = CMySettings::VpnConnection::WiFi;
			break;
		case IDC_RADIO_VPN_CONNECTION_ANY:
			ret = CMySettings::VpnConnection::Any;
			break;
		default:
			LOG4CXX_ERROR_FMT(logger, _T("Unknown RadioButton ID: %d"), checkedButtonId);
			break;
		}
	}
	return ret;
}
