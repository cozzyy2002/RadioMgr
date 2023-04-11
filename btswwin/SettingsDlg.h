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
template<> void Controller<int, CEdit>::setValue();
template<> void Controller<int, CEdit>::getValue();
template<> bool Controller<int, CEdit>::isChanged() const;

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
	CButton m_switchByLcdState;
	CButton m_restoreRadioState;
	CButton m_saveWindowPlacement;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClickedCheckButton();
	afx_msg void OnClickedSaveSettings();
	CEdit m_setRadioStateTimeout;
	CSpinButtonCtrl m_setRadioStateTimeoutSpin;
	afx_msg void OnEnChangeEditSetRadioStateTimeout();
};
