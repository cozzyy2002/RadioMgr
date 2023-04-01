// CSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "SettingsDlg.h"


// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialogEx)

CSettingsDlg::CSettingsDlg(CMySettings& settings, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETTINGS, pParent), m_settings(settings)
{

}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SWITCH_BY_LCD_STATE, m_switchByLcdState);
	DDX_Control(pDX, IDC_CHECK_RESTORE_RADIO_STATE, m_restoreRadioState);
	DDX_Control(pDX, IDC_CHECK_SAVE_WINDOW_PLACEMENT, m_saveWindowPlacement);
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSettingsDlg message handlers


BOOL CSettingsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_switchByLcdState.SetCheck(m_settings.switchByLcdState);
	m_restoreRadioState.SetCheck(m_settings.restoreRadioState);
	m_saveWindowPlacement.SetCheck(m_settings.saveWindowPlacement);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSettingsDlg::OnBnClickedOk()
{
	m_settings.switchByLcdState = m_switchByLcdState.GetCheck();
	m_settings.restoreRadioState = m_restoreRadioState.GetCheck();
	m_settings.saveWindowPlacement = m_saveWindowPlacement.GetCheck();

	CDialogEx::OnOK();
}
