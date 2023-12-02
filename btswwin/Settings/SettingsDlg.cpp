// CSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "SettingsDlg.h"
#include "BluetoothTabItem.h"
#include "NetworkTabItem.h"
#include "MiscTabItem.h"

// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

CSettingsDlg::CSettingsDlg(CMySettings& settings, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS, pParent), m_settings(settings)
	, SelectedTab(0)
{
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::updateUIState()
{
	// Set enabled state of [Save] and [OK] buttons.
	//   [Save] button is enabled when all values are valid and at least one value is changed.
	//   [OK] button is enabled when all values are valid.
	auto isValid = true;	// All setting values are valid.
	auto isChanged = false;	// At least one setting value is changed.
	for(auto& item : m_tabItems) {
		isValid &= item->isValid();
		isChanged |= item->isChanged();
	}

	GetDlgItem(ID_SAVE_SETTINGS)->EnableWindow((isValid && isChanged) ? TRUE : FALSE);
	GetDlgItem(IDOK)->EnableWindow(isValid ? TRUE : FALSE);
}

// Sets state of all controls to corresponding settings value.
void CSettingsDlg::applyChanges()
{
	for(auto& item : m_tabItems) {
		item->applyChanges();
	};
}

// Selects tab item.
// If specified tab index is out of range,
// the index is justified to the range by rotating fashion.
void CSettingsDlg::selectTab(int nTab)
{
	if(nTab < 0) nTab = (int)m_tabItems.size() - 1;
	else if(m_tabItems.size() <= nTab) nTab = 0;
	m_tabCtrl.SetCurSel(nTab);
	SelectedTab = nTab;

	OnTcnSelchangeTabSettings(nullptr, nullptr);
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_SETTINGS, m_tabCtrl);
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(ID_SAVE_SETTINGS, &CSettingsDlg::OnClickedSaveSettings)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SETTINGS, &CSettingsDlg::OnTcnSelchangeTabSettings)
	ON_MESSAGE(WM_TABITEM_VALUE_CHANGED, &CSettingsDlg::OnTabItemValueChanged)
END_MESSAGE_MAP()


// CSettingsDlg message handlers


BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Create DialogBoxes and attach them as tab items of CTabCtrl.
	m_tabItems.push_back(std::unique_ptr<CTabItem>(new CBluetoothTabItem(m_settings)));
	m_tabItems.push_back(std::unique_ptr<CTabItem>(new CNetworkTabItem(m_settings)));
	m_tabItems.push_back(std::unique_ptr<CTabItem>(new CMiscTabItem(m_settings)));
	// NOTE: Tab items should be inserted before retrieving position of them.
	for(int i = 0; i < m_tabItems.size(); i++) {
		auto item = m_tabItems[i].get();
		m_tabCtrl.InsertItem(i, item->getName());
	};
	CRect rect, tabRect;
	m_tabCtrl.GetWindowRect(&rect);
	m_tabCtrl.GetItemRect(0, &tabRect);
	m_tabCtrl.AdjustRect(FALSE, &rect);
	m_tabCtrl.ScreenToClient(&rect);
	rect.top += tabRect.Height();
	for(auto& item : m_tabItems) {
		item->Create(this);
		item->MoveWindow(&rect);
	};

	selectTab(SelectedTab);

	updateUIState();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSettingsDlg::OnTcnSelchangeTabSettings(NMHDR* pNMHDR, LRESULT* pResult)
{
	SelectedTab = m_tabCtrl.GetCurSel();
	for(int i = 0; i < m_tabItems.size(); i++) {
		m_tabItems[i]->ShowWindow((i == SelectedTab) ? SW_SHOW : SW_HIDE);
	}

	// Set focus to the tab control itself
	// to ensure it doesn't remain on the hidden tab item.
	m_tabCtrl.SetFocus();

	if(pResult) { *pResult = 0; }
}

static bool isKeyPressed(int nVertKey)
{
	return (GetKeyState(nVertKey) & 0x8000) ? true : false;
}

BOOL CSettingsDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN) {
		auto ctrl = isKeyPressed(VK_CONTROL);
		auto shift = isKeyPressed(VK_SHIFT);
		switch(pMsg->wParam) {
		case VK_TAB:
			if(ctrl) {
				// Process [Ctrl] + ([Shift] +) TAB to change selected tab.
				selectTab(SelectedTab + (shift ? -1 : 1));

				return TRUE;
			}
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CSettingsDlg::OnBnClickedOk()
{
	applyChanges();

	CDialogEx::OnOK();
}


void CSettingsDlg::OnClickedSaveSettings()
{
	applyChanges();

	if(m_settings.saveWindowPlacement)
	{
		m_settings.windowPlacement->length = sizeof(WINDOWPLACEMENT);
		WIN32_EXPECT(GetWindowPlacement(m_settings.windowPlacement));
	}

	m_settings.save();

	updateUIState();
}


afx_msg LRESULT CSettingsDlg::OnTabItemValueChanged(WPARAM wParam, LPARAM lParam)
{
	updateUIState();
	return 0;
}
