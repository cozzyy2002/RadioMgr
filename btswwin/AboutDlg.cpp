#include "pch.h"

#include "Resource.h"
#include "AboutDlg.h"

CAboutDlg::CAboutDlg(CResourceReader& resourceReader)
	: CDialogEx(IDD_ABOUTBOX)
	, m_resourceReader(resourceReader)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString str;
	str.Format(_T("About %s"), m_resourceReader.getProductName().GetString());
	SetWindowText(str);

	GetDlgItem(IDC_STATIC_COMPANY_NAME)->SetWindowText(m_resourceReader.getCompanyName());
	GetDlgItem(IDC_STATIC_COPYRIGHT)->SetWindowText(m_resourceReader.getLegalCopyright());
	GetDlgItem(IDC_STATIC_FILE_VERSION)->SetWindowText(m_resourceReader.getFileVersion());
	GetDlgItem(IDC_STATIC_PRODUCT_NAME)->SetWindowText(m_resourceReader.getProductName());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
