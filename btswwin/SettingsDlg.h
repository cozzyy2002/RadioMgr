#pragma once
#include "afxdialogex.h"

#include "MySettings.h"


class IController
{
public:
	virtual void setValue() = 0;
	virtual void getValue() = 0;
	virtual bool isChanged() const = 0;
};

template<typename T, class C>
class Controller : public IController
{
public:
	Controller(CSettings::Value<T>& value, C& ctrl) : value(value), ctrl(ctrl) {}

	virtual void setValue() override;
	virtual void getValue() override;
	virtual bool isChanged() const override;

protected:
	CSettings::Value<T>& value;
	C& ctrl;
};

template<> void Controller<BOOL, CButton>::setValue();
template<> void Controller<BOOL, CButton>::getValue();
template<> bool Controller<BOOL, CButton>::isChanged() const;

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

	void updateUIState();
	void applyChanges();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_switchByLcdState;
	CButton m_restoreRadioState;
	CButton m_saveWindowPlacement;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClickedCheckSwitchByLcdState();
	afx_msg void OnClickedSaveSettings();
};
