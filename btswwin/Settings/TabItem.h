#pragma once

#include "MySettings.h"

class IController;

#define WM_TABITEM_VALUE_CHANGED (WM_USER + 20)

class CTabItem : public CDialogEx
{
public:
	CTabItem(UINT nIDTemplate, LPCTSTR name, CMySettings& settings);
	virtual ~CTabItem() {}

	virtual BOOL Create(CWnd* pParent);
	virtual bool isChanged() const;
	virtual bool isValid() const { return true; }
	virtual void applyChanges();
	LPCTSTR getName() const { return m_name; }

protected:
	template<typename T, class C>
	void addController(CSettings::Value<T>& value, C& ctrl)
	{
		addController(new Controller<T, C>(value, ctrl));
	}
	void addController(IController* controller)
	{
		m_controllers.push_back(std::unique_ptr<IController>(controller));
	}

	void notifyValueChanged();

	CMySettings& m_settings;
	const CString m_name;
	std::vector<std::unique_ptr<IController>> m_controllers;
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


// Sets check state of the button.
inline void setButtonCheck(CButton& button, bool value) { button.SetCheck(value ? BST_CHECKED : BST_UNCHECKED); }
inline void setButtonCheck(CButton& button, const CSettings::Value<bool>& value) { setButtonCheck(button, (bool)value); }

// Returns TRUE if the button is checked, otherwise FALSE.
inline BOOL isButtonChecked(const CButton& button) { return (button.GetCheck() == BST_CHECKED) ? TRUE : FALSE; }

// Interface used to exchange setting value and state of control.
class IController
{
public:
	virtual ~IController() {}

	// Sets setting value to state of the control.
	virtual void setValueToCtrl() = 0;

	// Gets setting value from state of the control.
	virtual void getValueFromCtrl() = 0;

	// Returns true if one of following conditions is satisfied.
	//   State of the control is not set to the value yet.
	//   The value is changed but not saved to setting storage yet.
	virtual bool isChanged() const = 0;
};

template<typename T, class C>
class Controller : public IController
{
public:
	Controller(CSettings::Value<T>& value, C& ctrl) : value(value), ctrl(ctrl) {}

	virtual void setValueToCtrl() override;
	virtual void getValueFromCtrl() override;
	virtual bool isChanged() const override;

protected:
	CSettings::Value<T>& value;
	C& ctrl;
};
