
// btswwinDlg.h : header file
//

#pragma once

#include "RadioNotifyListener.h"
#include "RadioInstanceList.h"
#include "BluetoothDeviceList.h"
#include "../Common/SafeHandle.h"

#include <RadioMgr.h>
#include <atlbase.h>
#include <memory>
#include <thread>

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
	void print(const CTime&, LPCTSTR, ...);
	void printV(const CTime&, LPCTSTR, va_list);

protected:
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

	HRESULT OnRadioInstanceListContextMenu(CPoint point);
	bool CanSwitchRadio(DEVICE_RADIO_STATE state);
	bool SwitchRadio(DEVICE_RADIO_STATE state);
	HRESULT OnBluetoothDeviceListContextMenu(CPoint point);
	bool CanConnectDevice(const BLUETOOTH_DEVICE_INFO* deviceInfo = nullptr);
	HRESULT ConnectDevice(const BLUETOOTH_DEVICE_INFO* deviceInfo = nullptr);

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
	CListBox m_ListLog;
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData);
//	afx_msg void OnDestroy();
	afx_msg void OnBnClickedEditCopy();
	BOOL m_switchByLcdState;
	BOOL m_restoreRadioState;
//	virtual void OnFinalRelease();
protected:
	afx_msg LRESULT OnUserPrint(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRadioManagerNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserConnectDeviceResult(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateCommandUI(CCmdUI*);
	BOOL OnLocalRadioSwitchCommand(UINT);
	void OnConnectDeviceCommand() { ConnectDevice(); }
	void OnContextMenu(CWnd* pWnd, CPoint point);
	void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};
