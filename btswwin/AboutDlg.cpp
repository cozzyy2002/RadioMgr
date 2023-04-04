#include "pch.h"

#include "Resource.h"
#include "AboutDlg.h"

CAboutDlg::CAboutDlg(CResourceReader& resourceReader)
	: CDialogEx(IDD_ABOUTBOX)
	, m_resourceReader(resourceReader)
	, m_companyName(_T(""))
	, m_copyright(_T(""))
	, m_fileVersion(_T(""))
	, m_productName(_T(""))
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_COMPANY_NAME, m_companyName);
	DDX_Text(pDX, IDC_STATIC_COPYRIGHT, m_copyright);
	DDX_Text(pDX, IDC_STATIC_FILE_VERSION, m_fileVersion);
	//  DDX_Text(pDX, IDC_EDIT_GIT_COMMIT, m_gitCommit);
	DDX_Text(pDX, IDC_STATIC_PRODUCT_NAME, m_productName);
	DDX_Control(pDX, IDC_EDIT_GIT_COMMIT, m_gitCommit);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString str;
	str.Format(_T("About %s"), m_resourceReader.getProductName().GetString());
	SetWindowText(str);

	m_companyName = m_resourceReader.getCompanyName();
	m_copyright = m_resourceReader.getLegalCopyright();
	m_fileVersion = m_resourceReader.getFileVersion();
	m_productName = m_resourceReader.getProductName();

	// Set Git commit custom resource to multi-line CEdit control.
	// Note: Resource IDR_CUSTOM_STRING_GIT_COMMIT should be:
	//         Encoded by UTF-8(LPCSTR).
	//         consits of lines that end with CR/LF.
	// Command: git log -n 1 btswwin > btswwin\res\gitcommit.rc.txt
	m_gitCommit.SetWindowText(m_resourceReader.getCustomString<LPCSTR>(IDR_CUSTOM_STRING_GIT_COMMIT));

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
