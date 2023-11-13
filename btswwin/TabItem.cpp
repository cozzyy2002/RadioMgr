#include "pch.h"
#include "TabItem.h"
#include "../Common/Assert.h"

CTabItem::CTabItem(UINT nIDTemplate, LPCTSTR name, CMySettings& settings)
    : CDialogEx(nIDTemplate), m_name(name), m_settings(settings)
{
}

BOOL CTabItem::Create(CWnd* pParent)
{
    return CDialogEx::Create(m_lpszTemplateName, pParent);
}

BOOL CTabItem::isChanged() const
{
	for(auto& c : m_controllers) {
		if(c->isChanged()) {
			return TRUE;
		}
	}
	return FALSE;
}

void CTabItem::applyChanges()
{
	for(auto& c : m_controllers) {
		c->getValueFromCtrl();
	}
}


BOOL CTabItem::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set all setting values to corresponding control.
	for(auto& c : m_controllers) {
		c->setValueToCtrl();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CTabItem::notifyValueChanged()
{
	WIN32_EXPECT(
		::PostMessage(GetParent()->m_hWnd, WM_TABITEM_VALUE_CHANGED, 0, 0)
	);
}

// Returns true if all added controls are available.
bool CTabItem::areAllControlsAvailable() const
{
	for(auto& c : m_controllers) {
		if(!c->getCtrlWnd()->GetSafeHwnd()) { return false; }
	}
	return true;
}


#pragma region Controller<bool, CButton>
template<>
void Controller<bool, CButton>::setValueToCtrl()
{
	setButtonCheck(ctrl, value);
}

template<>
void Controller<bool, CButton>::getValueFromCtrl()
{
	value = isButtonChecked(ctrl) ? true : false;
}

template<>
bool Controller<bool, CButton>::isChanged() const
{
	auto buttonChecked = isButtonChecked(ctrl);
	auto bValue = (value ? TRUE : FALSE);
	return (buttonChecked != bValue) || value.isChanged();
}
#pragma endregion

#pragma region Controller<int, CEdit>
template<>
void Controller<int, CEdit>::setValueToCtrl()
{
	CString text;
	text.Format(_T("%d"), (int)value);
	ctrl.SetWindowText(text);
}

template<>
void Controller<int, CEdit>::getValueFromCtrl()
{
	CString text;
	ctrl.GetWindowText(text);
	value = _tstoi(text.GetString());
}

template<>
bool Controller<int, CEdit>::isChanged() const
{
	CString text;
	ctrl.GetWindowText(text);
	auto iValue = _tstoi(text.GetString());
	return (value != iValue) || value.isChanged();
}
#pragma endregion

#pragma region Controller<CString, CEdit>
template<>
void Controller<CString, CEdit>::setValueToCtrl()
{
	ctrl.SetWindowText(value->GetString());
}

template<>
void Controller<CString, CEdit>::getValueFromCtrl()
{
	CString text;
	ctrl.GetWindowText(text);
	value = text;
}

template<>
bool Controller<CString, CEdit>::isChanged() const
{
	CString text;
	ctrl.GetWindowText(text);
	return (value != text) || value.isChanged();
}
#pragma endregion
