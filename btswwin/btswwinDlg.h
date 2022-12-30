
// btswwinDlg.h : header file
//

#pragma once

#include "../Common/SafeHandle.h"

#include <RadioMgr.h>
#include <atlbase.h>

// CbtswwinDlg dialog
class CbtswwinDlg : public CDialogEx
{
// Construction
public:
	CbtswwinDlg(CWnd* pParent = nullptr);	// standard constructor
	HRESULT createRadioInstance();

	void print(const CString&);
	void print(LPCTSTR, ...);

protected:
	CComPtr<IMediaRadioManager> m_radioManager;
	CComPtr<IRadioInstance> m_radioInstance;

	static void unregisterPowerNotify(HPOWERNOTIFY);
	using PowerNotifyHandle = SafeHandle<HPOWERNOTIFY, unregisterPowerNotify>;
	PowerNotifyHandle m_hPowerNotify;

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
};
