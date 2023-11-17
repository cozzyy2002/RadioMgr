#pragma once
#include "afxdialogex.h"
#include "TabItem.h"

// CNetworkTabItem dialog

class CNetworkTabItem : public CTabItem
{
	DECLARE_DYNAMIC(CNetworkTabItem)

public:
	CNetworkTabItem(CMySettings& settings);
	virtual ~CNetworkTabItem();

	virtual bool isValid() const override;

protected:
	void updateUIState();

public:
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_NETWORK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CButton m_connectVpn;
	CEdit m_vpnName;
	afx_msg void OnChangeEdit();
	afx_msg void OnClickedCheckConnectVpn();
	afx_msg void OnClickedRadio();
	CEdit m_vpnConnectionDelay;
	CSpinButtonCtrl m_vpnConnectionDelaySpin;
	CEdit m_vpnConnectionRetry;
	CSpinButtonCtrl m_vpnConnectionRetrySpin;
};


class VpnConnectionController : public IController
{
public:
	VpnConnectionController(CNetworkTabItem* dlg, CMySettings& settings)
		: m_dlg(dlg), m_value(settings.vpnConnection) {}

	virtual void setValueToCtrl() override;
	virtual void getValueFromCtrl() override;
	virtual bool isChanged() const override;
	virtual CWnd* getCtrlWnd() const override { return &m_dlg->m_connectVpn; }

protected:
	CNetworkTabItem* m_dlg;
	CSettings::EnumValue<CMySettings::VpnConnection>& m_value;

	CMySettings::VpnConnection getVpnConnection() const;
};
