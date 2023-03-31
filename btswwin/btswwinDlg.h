
// btswwinDlg.h : header file
//

#pragma once

#include "RadioNotifyListener.h"
#include "RadioInstanceList.h"
#include "BluetoothDeviceList.h"
#include "Settings.h"
#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

#include <RadioMgr.h>
#include <atlbase.h>
#include <memory>
#include <thread>

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
#pragma endregion

protected:
	CSettings settings;
};

enum {
	WM_USER_PRINT = WM_USER + 1,	// Sent by CbtswwinDlg::printV() method.
	WM_USER_RADIO_MANAGER_NOTIFY,	// Sent by RadioNotifyListener to notify RadioManager evens.
	WM_USER_CONNECT_DEVICE_RESULT,	// Sent after connecting device.
};

// CbtswwinDlg dialog
class CbtswwinDlg : public CDialogEx
{
// Construction
public:
	CbtswwinDlg(CWnd* pParent = nullptr);	// standard constructor
	HRESULT createRadioManager();

	void print(const CString&);
	void print(LPCTSTR, ...);

protected:
	CMySettings m_settings;
	CComPtr<IMediaRadioManager> m_radioManager;

	static void unregisterPowerNotify(HPOWERNOTIFY);
	using PowerNotifyHandle = SafeHandle<HPOWERNOTIFY, unregisterPowerNotify>;
	PowerNotifyHandle m_hPowerNotify;
	DEVICE_RADIO_STATE m_radioState;

	CComPtr<RadioNotifyListener> m_radioNotifyListener;

	HRESULT setRadioState(DEVICE_RADIO_STATE, bool restore = false);
	HRESULT setRadioState(RadioInstanceData& data, DEVICE_RADIO_STATE, bool restore = false);

	static const UINT_PTR PollingTimerId = 1;
	HRESULT checkRadioState();
	HRESULT checkBluetoothDevice();

	// Worker thread to connect to the device.
	std::unique_ptr<std::thread> m_connectDeviceThread;

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
	BOOL m_switchByLcdState;
	BOOL m_restoreRadioState;
//	virtual void OnFinalRelease();
protected:
	afx_msg LRESULT OnUserPrint(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRadioManagerNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserConnectDeviceResult(WPARAM wParam, LPARAM lParam);
	//virtual void PostNcDestroy();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSwitchRadioUpdateCommandUI(CCmdUI*);
	afx_msg BOOL OnSwitchRadioCommand(UINT);
	void OnConnectDeviceUpdateCommandUI(CCmdUI*);
	void OnConnectDeviceCommand();
	void OnCopyDeviceList() { HR_EXPECT_OK(m_bluetoothDevices.Copy()); }
	void OnCopyRadioList() { HR_EXPECT_OK(m_radioInstances.Copy()); }
	void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//virtual BOOL DestroyWindow();
	afx_msg void OnDestroy();
};
