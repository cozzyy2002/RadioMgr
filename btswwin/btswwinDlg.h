
// btswwinDlg.h : header file
//

#pragma once

#include "RadioNotifyListener.h"
#include "RadioInstanceList.h"
#include "BluetoothDeviceList.h"
#include "WLan.h"
#include "Net.h"
#include "RasDial.h"
#include "MySettings.h"
#include "ResourceReader.h"

#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

#include <RadioMgr.h>
#include <atlbase.h>
#include <memory>
#include <thread>

enum {
	WM_USER_PRINT = WM_USER + 1,	// Sent by CbtswwinDlg::printV() method.
	WM_USER_RADIO_MANAGER_NOTIFY,	// Sent by RadioNotifyListener to notify RadioManager evens.
	WM_USER_CONNECT_DEVICE_RESULT,	// Sent after connecting device.
	WM_USER_WLAN_NOTIFY,			// Sent by CWLan to notify Wi-Fi is connected/disconnected.
	WM_USER_NET_NOTIFY,				// Sent by CNet to notify connectivity changed.
	WM_USER_VPN_NOTIFY,				// Sent by CRasDial to notify result of connecting VPN.
};

// Deleter for MAllocPtr.
struct MAllocDeleter
{
	void operator ()(LPTSTR p) { free(p); }
};

// unique_ptr<> for string allocated by malloc() function,
// such as string returned by _tcsdup() function.
//   MAllocPtr str(_tcsdup(_T("String ...")));
using MAllocPtr = std::unique_ptr<TCHAR, MAllocDeleter>;

// CbtswwinDlg dialog
class CbtswwinDlg : public CDialogEx
{
// Construction
public:
	CbtswwinDlg(CResourceReader& resourceReader, CWnd* pParent = nullptr);	// standard constructor
	HRESULT createRadioManager();

	void print(const CString&);
	void print(LPCTSTR, ...);

protected:
	std::unique_ptr<CMySettings> m_settings;
	CResourceReader& m_resourceReader;
	CComPtr<IMediaRadioManager> m_radioManager;

	static void unregisterPowerNotify(HPOWERNOTIFY);
	using PowerNotifyHandle = SafeHandle<HPOWERNOTIFY, unregisterPowerNotify>;
	PowerNotifyHandle m_hPowerNotify;
	DEVICE_RADIO_STATE m_radioState;

	CComPtr<RadioNotifyListener> m_radioNotifyListener;

	std::vector<MAllocPtr> m_logFileList;

	HRESULT setRadioState(DEVICE_RADIO_STATE, bool restore = false);
	HRESULT setRadioState(RadioInstanceData& data, DEVICE_RADIO_STATE, bool restore = false);

	static const UINT_PTR PollingTimerId = 1;
	HRESULT checkRadioState();
	HRESULT checkBluetoothDevice();
	void resetThread(std::unique_ptr<std::thread>&);

	// Worker thread to connect to the device.
	std::unique_ptr<std::thread> m_connectDeviceThread;

	// Worker thread to execute setRadioState after delay.
	std::unique_ptr<std::thread> m_setRadioOnThread;

	/*
	 * Workaround that RadioInstance is removed and added when LID is opened sometimes.

		2023-06-01 09:36:33.225 [INFO] LIDSWITCH_STATE_CHANGE: LID is opened(1)
		2023-06-01 09:36:35.178 [INFO] InstanceRemove: Bluetooth_4c034fd052b8, UNKNOWN(-1)
		2023-06-01 09:36:36.889 [INFO] InstanceAdd: Bluetooth_4c034fd052b8:Bluetooth, DRS_SW_RADIO_OFF(1)

	 * RadioState is added to m_previousRadioStates when RadioInstance is removed.
	 * And it is used to restore state when RadioInstance is added.
	 * Then setRadioState() is called with restored state.
	 * 
	 * See OnUserRadioManagerNotify() method for details.
	 */
	struct RadioState {
		DEVICE_RADIO_STATE sate;
		BOOL isChecked;
	};
	std::map<CString, RadioState> m_previousRadioStates;

	CWLan m_wlan;
	CComPtr<CNet> m_net;
	CRasDial m_rasDial;

	CString m_wlanConnectedSsid;
	bool m_wlanIsSecured;
	bool m_netIsConnected;
	bool m_lidIsOpened;
	HRESULT connectVpn();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BTSWWIN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CRadioInstanceList m_radioInstances;
	CBluetoothDeviceList m_bluetoothDevices;
	CStatic m_StatusText;
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData);
//	afx_msg void OnDestroy();
//	virtual void OnFinalRelease();
protected:
	afx_msg LRESULT OnUserPrint(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRadioManagerNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserConnectDeviceResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserWLanNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserNetNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserVpnNotify(WPARAM wParam, LPARAM lParam);
	//virtual void PostNcDestroy();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSwitchRadioUpdateCommandUI(CCmdUI*);
	afx_msg BOOL OnSwitchRadioCommand(UINT);
	void OnConnectDeviceUpdateCommandUI(CCmdUI*);
	void OnConnectDeviceCommand();
	void OnDevicePropertiesUpdateCommandUI(CCmdUI*);
	void OnDevicePropertiesCommand();
	// Called when system menu is about to open.
	void OnExitUpdateCommandUI(CCmdUI*);
	void OnCopyDeviceList() { HR_EXPECT_OK(m_bluetoothDevices.Copy()); }
	void OnCopyRadioList() { HR_EXPECT_OK(m_radioInstances.Copy()); }
	void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//virtual BOOL DestroyWindow();
	afx_msg void OnDestroy();
	afx_msg void OnFileSettings();
	afx_msg void OnFileOpenLogCommandUI(CCmdUI*);
	afx_msg void OnFileOpenLog(UINT);
};
