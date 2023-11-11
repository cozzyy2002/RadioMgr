// MiscTabItem.cpp : implementation file
//

#include "pch.h"
#include "btswwin.h"
#include "afxdialogex.h"
#include "MiscTabItem.h"


// CMiscTabItem dialog

IMPLEMENT_DYNAMIC(CMiscTabItem, CDialogEx)

CMiscTabItem::CMiscTabItem(CMySettings& settings)
	: CTabItem(IDD_SETTINGS_MISC, _T("Miscellaneous"), settings)
{
	addController(m_settings.saveWindowPlacement, m_saveWindowPlacement);
}

CMiscTabItem::~CMiscTabItem()
{
}

void CMiscTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SAVE_WINDOW_PLACEMENT, m_saveWindowPlacement);
}


BEGIN_MESSAGE_MAP(CMiscTabItem, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_SAVE_WINDOW_PLACEMENT, &CMiscTabItem::OnClickedCheckButton)
END_MESSAGE_MAP()


// CMiscTabItem message handlers


BOOL CMiscTabItem::OnInitDialog()
{
	CTabItem::onInitDialog();

	return 0;
}

void CMiscTabItem::OnClickedCheckButton()
{
	//updateUIState();
}
