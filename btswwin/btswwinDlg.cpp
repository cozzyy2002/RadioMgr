
// btswwinDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "btswwin.h"
#include "btswwinDlg.h"
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

#pragma region ValueName template class

template<typename T>
struct ValueName {
	LPCTSTR name;
	T value;

	static LPCTSTR ValueFormat;
	template<size_t size>
	static CString getName(const ValueName(&list)[size], const T& v);
	static CString valueToString(const T& v);
};

template<typename T> LPCTSTR ValueName<T>::ValueFormat = _T("%d");
template<> LPCTSTR ValueName<UINT>::ValueFormat = _T("%u");
template<> LPCTSTR ValueName<float>::ValueFormat = _T("%f");

template<typename T> template<size_t size>
/*static*/ CString ValueName<T>::getName(const ValueName (&list)[size], const T& v)
{
	auto name = _T("UNKNOWN");
	for(auto& i : list) {
		if(i.value == v) {
			name = i.name;
			break;
		}
	}
	CString ret;
	ret.Format(_T("%s(%s)"), name, valueToString(v).GetString());
	return ret;
}

template<typename T>
/*static*/ CString ValueName<T>::valueToString(const T& v)
{
	CString ret;
	ret.Format(ValueFormat, v);
	return ret;
}

template<>
/*static*/ CString ValueName<GUID>::valueToString(const GUID& v)
{
	OLECHAR strGuid[50] = _T("");
	HR_EXPECT(0 < StringFromGUID2(v, strGuid, ARRAYSIZE(strGuid)), HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
	return strGuid;
}

#define VALUE_NAME_ITEM(x) {_T(#x), x}

#pragma endregion

CbtswwinDlg::CbtswwinDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BTSWWIN_DIALOG, pParent)
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
	if(10 < m_ListLog.GetCount()) {
		m_ListLog.DeleteString(0);
	}

	va_list args;
	va_start(args, fmt);
	CString text;
	text.FormatV(fmt, args);
	va_end(args);

	CTime now(CTime::GetCurrentTime());
	text = now.Format("%F %T ") + text;

	m_ListLog.AddString(text);
}

void CbtswwinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, ID_LIST_LOG, m_ListLog);
}

BEGIN_MESSAGE_MAP(CbtswwinDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDO_ON, &CbtswwinDlg::OnBnClickedOn)
	ON_BN_CLICKED(IDO_OFF, &CbtswwinDlg::OnBnClickedOff)
	ON_WM_POWERBROADCAST()
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

	HR_EXPECT_OK(CoInitializeEx(NULL, COINIT_MULTITHREADED));
	auto hr = createRadioInstance();
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
	HR_ASSERT_OK(col->GetAt(0, &m_radioInstance));

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
	auto hr = m_radioInstance->SetRadioState(DRS_RADIO_ON, 1);
	print(_T("Bluetooth ON: 0x%08x"), hr);
}



void CbtswwinDlg::OnBnClickedOff()
{
	auto hr = m_radioInstance->SetRadioState(DRS_SW_RADIO_OFF, 1);
	print(_T("Bluetooth OFF: 0x%08x"), hr);
}


UINT CbtswwinDlg::OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData)
{
	static const ValueName<UINT> powerEvents[] = {
		VALUE_NAME_ITEM(PBT_APMPOWERSTATUSCHANGE),
		VALUE_NAME_ITEM(PBT_APMRESUMEAUTOMATIC),
		VALUE_NAME_ITEM(PBT_APMRESUMESUSPEND),
		VALUE_NAME_ITEM(PBT_APMSUSPEND),
		VALUE_NAME_ITEM(PBT_POWERSETTINGCHANGE),
	};

	print(_T(__FUNCTION__) _T(": PowerEvent=%s"), ValueName<UINT>::getName(powerEvents, nPowerEvent));

	if(nPowerEvent == PBT_POWERSETTINGCHANGE) {
		static const ValueName<GUID> powerSettingGuids[] = {
			VALUE_NAME_ITEM(GUID_ACDC_POWER_SOURCE),
			VALUE_NAME_ITEM(GUID_BATTERY_PERCENTAGE_REMAINING),
			VALUE_NAME_ITEM(GUID_CONSOLE_DISPLAY_STATE),
			VALUE_NAME_ITEM(GUID_GLOBAL_USER_PRESENCE),
			VALUE_NAME_ITEM(GUID_IDLE_BACKGROUND_TASK),
			VALUE_NAME_ITEM(GUID_MONITOR_POWER_ON),
			VALUE_NAME_ITEM(GUID_POWER_SAVING_STATUS),
			VALUE_NAME_ITEM(GUID_POWERSCHEME_PERSONALITY),
			VALUE_NAME_ITEM(GUID_SESSION_DISPLAY_STATUS),
			VALUE_NAME_ITEM(GUID_SESSION_USER_PRESENCE),
			VALUE_NAME_ITEM(GUID_SYSTEM_AWAYMODE),
		};

		auto setting = (PPOWERBROADCAST_SETTING)nEventData;
		print(_T("  PowerSetting=%s"), ValueName<GUID>::getName(powerSettingGuids, setting->PowerSetting));
	}

	return CDialogEx::OnPowerBroadcast(nPowerEvent, nEventData);
}
