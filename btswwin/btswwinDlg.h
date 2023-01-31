
// btswwinDlg.h : header file
//

#pragma once

#include "RadioNotifyListener.h"
#include "../Common/SafeHandle.h"

#include <RadioMgr.h>
#include <atlbase.h>

enum {
	WM_USER_PRINT = WM_USER + 1,	// Sent by CbtswwinDlg::printV() method.
	WM_USER_RADIO_MANAGER_NOTIFY,	// Sent by RadioNotifyListener to notify RadioManager evens.
};

// CbtswwinDlg dialog
class CbtswwinDlg : public CDialogEx
{
// Construction
public:
	CbtswwinDlg(CWnd* pParent = nullptr);	// standard constructor
	HRESULT createRadioInstance();

	void print(const CString&);
	void print(LPCTSTR, ...);
	void print(const CTime&, LPCTSTR, ...);
	void printV(const CTime&, LPCTSTR, va_list);

protected:
	CComPtr<IMediaRadioManager> m_radioManager;
	CComPtr<IRadioInstance> m_radioInstance;
	CString m_radioInstanceId;

	static void unregisterPowerNotify(HPOWERNOTIFY);
	using PowerNotifyHandle = SafeHandle<HPOWERNOTIFY, unregisterPowerNotify>;
	PowerNotifyHandle m_hPowerNotify;
	DEVICE_RADIO_STATE m_radioState;

	CComPtr<RadioNotifyListener> m_radioNotifyListener;

	HRESULT setRadioState(DEVICE_RADIO_STATE);

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
	CListBox m_ListLog;
	afx_msg void OnBnClickedOn();
	afx_msg void OnBnClickedOff();
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData);
//	afx_msg void OnDestroy();
	afx_msg void OnBnClickedEditCopy();
	BOOL m_switchByLcdState;
	BOOL m_restoreRadioState;
//	virtual void OnFinalRelease();
protected:
	afx_msg LRESULT OnUserPrint(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRadioManagerNotify(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
};
