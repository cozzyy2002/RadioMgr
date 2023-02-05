
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
	CString* text = new CString();
	text->FormatV(fmt, args);
	*text = now.Format("%F %T ") + *text;
	if(!PostMessage(WM_USER_PRINT, 0, (LPARAM)text)) {
		delete text;
		CString err;
		err.Format(_T(__FUNCTION__ ": PostMessage(%d) failed. Error=%d\n"), WM_USER_PRINT, GetLastError());
		OutputDebugString(err.GetString());
	}
}

afx_msg LRESULT CbtswwinDlg::OnUserPrint(WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<CString> text((CString*)lParam);

	if(100 < m_ListLog.GetCount()) {
		m_ListLog.DeleteString(0);
	}
	auto index = m_ListLog.AddString(*text);
	m_ListLog.SetTopIndex(index);

	return 0;
}


void CbtswwinDlg::unregisterPowerNotify(HPOWERNOTIFY h)
{
	WIN32_EXPECT(UnregisterPowerSettingNotification(h));
}

void CbtswwinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RADIO_INSTANCES, m_radioInstances);
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
	ON_MESSAGE(WM_USER_PRINT, &CbtswwinDlg::OnUserPrint)
	ON_MESSAGE(WM_USER_RADIO_MANAGER_NOTIFY, &CbtswwinDlg::OnUserRadioManagerNotify)
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

	// Prepare for AssertFailedProc() static function.
	::pdlg = this;
	tsm::Assert::onAssertFailedProc = ::AssertFailedProc;

	m_radioInstances.OnInitCtrl();

	auto hr = createRadioInstance();

	if(SUCCEEDED(hr)) {
		m_hPowerNotify = RegisterPowerSettingNotification(m_hWnd, &GUID_LIDSWITCH_STATE_CHANGE, DEVICE_NOTIFY_WINDOW_HANDLE);
		hr = WIN32_EXPECT(m_hPowerNotify);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// Create IMedioRadioManager object for Bluetooth device and setup RadioNotifyListener.
HRESULT CbtswwinDlg::createRadioInstance()
{
	// Create IMediaRadioManager for Bluetooth.
	static const CLSID clsid = { 0xafd198ac, 0x5f30, 0x4e89, { 0xa7, 0x89, 0x5d, 0xdf, 0x60, 0xa6, 0x93, 0x66 } };
	HR_ASSERT_OK(m_radioManager.CoCreateInstance(clsid));

	// Get IConnectionPoint object and set RadioNotifyListener as IMediaRadioManagerNotifySink.
	CComPtr<IConnectionPointContainer> cpc;
	HR_ASSERT_OK(m_radioManager.QueryInterface(&cpc));
	CComPtr<IConnectionPoint> cp;
	HR_ASSERT_OK(cpc->FindConnectionPoint(IID_IMediaRadioManagerNotifySink, &cp));
	m_radioNotifyListener = new RadioNotifyListener(m_hWnd, WM_USER_RADIO_MANAGER_NOTIFY);
	HR_ASSERT_OK(m_radioNotifyListener->advise(cp));

	return S_OK;
}

void CbtswwinDlg::PostNcDestroy()
{
	// AssertFailedProc() of this dialog no longer works.
	tsm::Assert::onAssertFailedProc = nullptr;

	// Disconnect RadioNotifyListener from IConnectionPoint.
	if(m_radioNotifyListener) {
		HR_EXPECT_OK(m_radioNotifyListener->unadvise());
	}

	CDialogEx::PostNcDestroy();
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
			static const ValueName<UCHAR> lidSwitchDatas[] = {
				{0, _T("closed")},
				{1, _T("opened")},
			};
			print(_T("LIDSWITCH_STATE_CHANGE: LID is %s"), ValueToString(lidSwitchDatas, setting->Data[0]).GetString());

			UpdateData();
			if(m_switchByLcdState) {
				switch(setting->Data[0]) {
				case 0:		// The lid is closed.
					if(m_restoreRadioState) {
						// Save current state to restore when lid will be opened.
						m_radioInstances.For([](RadioInstanceData& data)
							{
								data.savedState = data.state;
								return S_OK;
							}
						);
					}
					setRadioState(DRS_SW_RADIO_OFF);
					break;;
				case 1:		// The lid is opened.
					setRadioState(DRS_RADIO_ON, m_restoreRadioState);
					break;
				}
			}
		}
	}

	return TRUE;
}

HRESULT CbtswwinDlg::setRadioState(DEVICE_RADIO_STATE state, bool restore /*= false*/)
{
	return m_radioInstances.For([state, restore](RadioInstanceData& data)
		{
			auto newState = restore ? data.savedState : state;
			return (data.state != newState) ?
				HR_EXPECT_OK(data.radioInstance->SetRadioState(newState, 1)) :
				S_FALSE;
		}
	);
}

// Process message sent by RadioNotifyListener
afx_msg LRESULT CbtswwinDlg::OnUserRadioManagerNotify(WPARAM wParam, LPARAM lParam)
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

	std::unique_ptr<RadioNotifyListener::Message> message((RadioNotifyListener::Message*)lParam);
	CString type(_T("Unknown"));
	CString name(_T("Unknown"));
	auto state = (DEVICE_RADIO_STATE)(-1);
	switch(message->type) {
	case RadioNotifyListener::Message::Type::InstanceAdd:
		// RadioNotifyListener::OnInstanceAdd(IRadioInstance* pRadioInstance)
		{
			type = _T("Add");
			const RadioInstanceData* pData = nullptr;
			m_radioInstances.Add(message->radioInstance, &pData);
			name.Format(_T("%s:%s"), pData->name.GetString(), pData->id.GetString());
			state = pData->state;
		}
		break;
	case RadioNotifyListener::Message::Type::InstanceRemove:
		type = _T("Remove");
		// RadioNotifyListener::OnInstanceRemove(BSTR bstrRadioInstanceId)
		name = message->radioInstanceId;
		m_radioInstances.Remove(name);
		break;
	case RadioNotifyListener::Message::Type::InstanceRadioChange:
		type = _T("InstanceRadioChange");
		// RadioNotifyListener::OnInstanceRadioChange(BSTR bstrRadioInstanceId, DEVICE_RADIO_STATE radioState)
		name = message->radioInstanceId;
		state = message->radioState;
		m_radioInstances.StateChange(name, state);
		break;
	}

	print(_T("%s: %s, %s"), type.GetString(), name.GetString(), ValueToString(states, state).GetString());
	return 0;
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
	auto hMem = GlobalAlloc(GMEM_MOVEABLE, size);
	if(SUCCEEDED(WIN32_EXPECT(hMem != NULL))) {
		memcpy_s(GlobalLock(hMem), size, text.LockBuffer(), size);
		WIN32_EXPECT(GlobalUnlock(hMem));
		text.UnlockBuffer();

		UINT format = (sizeof(TCHAR) == sizeof(WCHAR) ? CF_UNICODETEXT : CF_TEXT);
		WIN32_EXPECT(::SetClipboardData(format, hMem) == hMem);
	} else {
		print(_T("Failed to allocate %d bytes memory"), size);
	}
	CloseClipboard();
}
