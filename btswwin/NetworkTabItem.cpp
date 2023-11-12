// NetworkTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "NetworkTabItem.h"


// CNetworkTabItem dialog

IMPLEMENT_DYNAMIC(CNetworkTabItem, CDialogEx)

CNetworkTabItem::CNetworkTabItem(CMySettings& settings)
	: CTabItem(IDD_SETTINGS_NETWORK, _T("Network"), settings)
{
	addController(m_settings.vpnName, m_vpnName);
}

CNetworkTabItem::~CNetworkTabItem()
{
}

void CNetworkTabItem::updateUIState()
{
	// Set enabled state of the controls depending on whether [Connect VPN] is checked.
	static const int ids[] = {
		IDC_EDIT_VPN_NAME,
		IDC_RADIO_VPN_CONNECTION,IDC_RADIO_VPN_CONNECTION_WIFI, IDC_RADIO_VPN_CONNECTION_ANY
	};
	auto enable = isButtonChecked(m_connectVpn);
	for(auto id : ids) {
		GetDlgItem(id)->EnableWindow(enable);
	}

	notifyValueChanged();
}

void CNetworkTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_CONNECT_VPN, m_connectVpn);
	DDX_Control(pDX, IDC_EDIT_VPN_NAME, m_vpnName);
	DDX_Control(pDX, IDC_RADIO_VPN_CONNECTION, m_vpnConnection);
}

BOOL CNetworkTabItem::OnInitDialog()
{
	CTabItem::onInitDialog();

	return 0;
}


BEGIN_MESSAGE_MAP(CNetworkTabItem, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_VPN_CONNECTION, &CNetworkTabItem::OnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_VPN_CONNECTION_WIFI, &CNetworkTabItem::OnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_VPN_CONNECTION_ANY, &CNetworkTabItem::OnClickedRadio)
	ON_EN_CHANGE(IDC_EDIT_VPN_NAME, &CNetworkTabItem::OnChangeEdit)
	ON_BN_CLICKED(IDC_CHECK_CONNECT_VPN, &CNetworkTabItem::OnClickedCheck)
END_MESSAGE_MAP()


// CNetworkTabItem message handlers


void CNetworkTabItem::OnClickedCheck()
{
	updateUIState();
}


void CNetworkTabItem::OnChangeEdit()
{
	// Confirm that window handle of all controls are available
	// to not call updateUIState() method inside constructor.
	if(!areAllControlsAvailable()) { return; }

	updateUIState();
}


void CNetworkTabItem::OnClickedRadio()
{
	updateUIState();
}
