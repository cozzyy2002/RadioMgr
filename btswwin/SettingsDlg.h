#pragma once
#include "afxdialogex.h"

#include "MySettings.h"

class CTabItem;

// Interface used to exchange setting value and state of control.
class IController
{
public:
	// Sets setting value to state of the control.
	virtual void setValueToCtrl() = 0;

	// Gets setting value from state of the control.
	virtual void getValueFromCtrl() = 0;

	// Returns true if one of following conditions is satisfied.
	//   State of the control is not set to the value yet.
	//   The value is changed but not saved to setting storage yet.
	virtual bool isChanged() const = 0;

	// Returns CWnd of the control.
	virtual CWnd* getCtrlWnd() const = 0;
};

template<typename T, class C>
class Controller : public IController
{
public:
	Controller(CSettings::Value<T>& value, C& ctrl) : value(value), ctrl(ctrl) {}

	virtual void setValueToCtrl() override;
	virtual void getValueFromCtrl() override;
	virtual bool isChanged() const override;
	virtual CWnd* getCtrlWnd() const override { return &ctrl; }

protected:
	CSettings::Value<T>& value;
	C& ctrl;
};

// CSettingsDlg dialog

class CSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CMySettings& settings, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSettingsDlg();

protected:
	CMySettings& m_settings;
	std::vector<std::unique_ptr<IController>> m_controllers;
	std::vector<std::unique_ptr<CTabItem>> m_tabItems;

	void updateUIState();
	void applyChanges();

	template<typename T, class C>
	void addController(CSettings::Value<T>& value, C& ctrl)
	{
		m_controllers.push_back(std::unique_ptr<IController>(new Controller<T, C>(value, ctrl)));
	}

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClickedCheckButton();
	afx_msg void OnClickedSaveSettings();
	afx_msg void OnEnChangeEdit();
	CTabCtrl m_tabCtrl;
	afx_msg void OnTcnSelchangeTabSettings(NMHDR* pNMHDR, LRESULT* pResult);
};
