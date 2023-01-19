
// btswwinDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "btswwin.h"
#include "btswwinDlg.h"
#include "ValueName.h"
#include "../Common/Assert.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CbtswwinDlg dialog

CbtswwinDlg::CbtswwinDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BTSWWIN_DIALOG, pParent)
	, m_radioState(DRS_RADIO_INVALID)
	, m_switchByLcdState(TRUE)
	, m_restoreRadioState(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

static CbtswwinDlg* pdlg = nullptr;

// Show failed message in log list control with formatted HRESULT.
static void AssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	if(pdlg) {
		LPTSTR msg;
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		auto formatResult = FormatMessage(flags, NULL, hr, 0, (LPTSTR)&msg, 100, NULL);
		if(formatResult) {
			pdlg->print(_T("`%s` failed: %s(0x%08x)"), exp, msg, hr);
			LocalFree(msg);
			return;
		}
	}

	// Call default proc if CbtswwinDlg is not created yet or FormatMessage() failed.
	tsm::Assert::defaultAssertFailedProc(hr, exp, sourceFile, line);
}

void CbtswwinDlg::print(const CString& text)
{
	print(text.GetString());
}

void CbtswwinDlg::print(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printV(CTime::GetCurrentTime(), fmt, args);
	va_end(args);
}

void CbtswwinDlg::print(const CTime& now, LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printV(now, fmt, args);
	va_end(args);
}

void CbtswwinDlg::printV(const CTime& now, LPCTSTR fmt, va_list args)
{
	CString text;
	text.FormatV(fmt, args);
	text = now.Format("%F %T ") + text;

	if(100 < m_ListLog.GetCount()) {
		m_ListLog.DeleteString(0);
	}
	m_ListLog.AddString(text);
}

void CbtswwinDlg::unregisterPowerNotify(HPOWERNOTIFY h)
{
	WIN32_EXPECT(UnregisterPowerSettingNotification(h));
}

void CbtswwinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, ID_LIST_LOG, m_ListLog);
	DDX_Check(pDX, IDC_CHECK_SWITCH_BY_LCD_STATE, m_switchByLcdState);
	DDX_Check(pDX, IDC_CHECK_RESTORE_RADIO_STATE, m_restoreRadioState);
}

BEGIN_MESSAGE_MAP(CbtswwinDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDO_ON, &CbtswwinDlg::OnBnClickedOn)
	ON_BN_CLICKED(IDO_OFF, &CbtswwinDlg::OnBnClickedOff)
	ON_WM_POWERBROADCAST()
	ON_BN_CLICKED(ID_EDIT_COPY, &CbtswwinDlg::OnBnClickedEditCopy)
END_MESSAGE_MAP()


// CbtswwinDlg message handlers

BOOL CbtswwinDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// Prepare for AssertFailedProc() static function.
	::pdlg = this;
	tsm::Assert::onAssertFailedProc = ::AssertFailedProc;

	auto hr = createRadioInstance();

	if(SUCCEEDED(hr)) {
		m_hPowerNotify = RegisterPowerSettingNotification(m_hWnd, &GUID_LIDSWITCH_STATE_CHANGE, DEVICE_NOTIFY_WINDOW_HANDLE);
		hr = WIN32_EXPECT(m_hPowerNotify);
	}

	print(_T("%s to initialize"), SUCCEEDED(hr) ? _T("Succeeded") : _T("Failed"));
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// Create IRadioInstance object for Bluetooth device.
HRESULT CbtswwinDlg::createRadioInstance()
{
	CLSID clsid;
	HR_ASSERT_OK(CLSIDFromString(L"{afd198ac-5f30-4e89-a789-5ddf60a69366}", &clsid));
	HR_ASSERT_OK(m_radioManager.CoCreateInstance(clsid));
	CComPtr<IRadioInstanceCollection> col;
	HR_ASSERT_OK(m_radioManager->GetRadioInstances(&col));
	UINT32 instanceCount;
	HR_ASSERT_OK(col->GetCount(&instanceCount));
	HR_ASSERT(0 < instanceCount, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
	HR_ASSERT_OK(col->GetAt(0, &m_radioInstance));
	HR_ASSERT_OK(m_radioInstance->GetRadioState(&m_radioState));

	return S_OK;
}

void CbtswwinDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CbtswwinDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CbtswwinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CbtswwinDlg::OnBnClickedOn()
{
	setRadioState(DRS_RADIO_ON);
}



void CbtswwinDlg::OnBnClickedOff()
{
	setRadioState(DRS_SW_RADIO_OFF);
}


UINT CbtswwinDlg::OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData)
{
	if(nPowerEvent == PBT_POWERSETTINGCHANGE) {
		auto setting = (PPOWERBROADCAST_SETTING)nEventData;
		if(setting->PowerSetting == GUID_LIDSWITCH_STATE_CHANGE) {
			UpdateData();
			if(m_switchByLcdState) {
				switch(setting->Data[0]) {
				case 0:		// The lid is closed.
					if(m_restoreRadioState) {
						HR_EXPECT_OK(m_radioInstance->GetRadioState(&m_radioState));
					}
					setRadioState(DRS_SW_RADIO_OFF);
					break;;
				case 1:		// The lid is opened.
				{
					auto newState = (m_restoreRadioState ? m_radioState : DRS_RADIO_ON);
					setRadioState(newState);
					break;
				}
				}
			}
		}
	}

	return TRUE;
}

HRESULT CbtswwinDlg::setRadioState(DEVICE_RADIO_STATE newState)
{
	static const ValueName<DEVICE_RADIO_STATE> states[] = {
		VALUE_NAME_ITEM(DRS_RADIO_ON),
		VALUE_NAME_ITEM(DRS_SW_RADIO_OFF),
		VALUE_NAME_ITEM(DRS_HW_RADIO_OFF),
		VALUE_NAME_ITEM(DRS_SW_HW_RADIO_OFF),
		VALUE_NAME_ITEM(DRS_HW_RADIO_ON_UNCONTROLLABLE),
		VALUE_NAME_ITEM(DRS_RADIO_INVALID),
		VALUE_NAME_ITEM(DRS_HW_RADIO_OFF_UNCONTROLLABLE),
	};

	CTime now(CTime::GetCurrentTime());
	DEVICE_RADIO_STATE currentState;
	HR_ASSERT_OK(m_radioInstance->GetRadioState(&currentState));
	auto hr = S_FALSE;
	if(currentState != newState) {
		hr = HR_EXPECT_OK(m_radioInstance->SetRadioState(newState, 1));
		print(now, _T("Bluetooth %s -> %s: 0x%x"), ValueToString(states, currentState).GetString(), ValueToString(states, newState).GetString(), hr);
	}
	return hr;
}

// Copy text in the log window to clipboard.
void CbtswwinDlg::OnBnClickedEditCopy()
{
	auto line = m_ListLog.GetCount();
	CString text;
	for(int i = 0; i < line; i++) {
		CString t;
		m_ListLog.GetText(i, t);
		text += (t + _T("\n"));
	}

	OpenClipboard();
	EmptyClipboard();

	size_t size = (text.GetLength() + 1) * sizeof(TCHAR);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
	if(SUCCEEDED(WIN32_EXPECT(hMem))) {
		memcpy_s(GlobalLock(hMem), size, text.LockBuffer(), size);
		GlobalUnlock(hMem);
		text.UnlockBuffer();

		UINT format = (sizeof(TCHAR) == sizeof(WCHAR) ? CF_UNICODETEXT : CF_TEXT);
		WIN32_EXPECT(::SetClipboardData(format, hMem) == hMem);
		print(_T("Copied %d line(%d bytes) to Clipboard"), line, size);
	} else {
		print(_T("Failed to allocate %d bytes memory"), size);
	}
	CloseClipboard();
}
