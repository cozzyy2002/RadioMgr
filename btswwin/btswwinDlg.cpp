
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

#define DEBUG_LIDSWITCH
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

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CbtswinDlg")));

// Show failed message in log list control with formatted HRESULT.
static void AssertFailedProc(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	CString _msg;
	LPTSTR msg;
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	auto formatResult = FormatMessage(flags, NULL, hr, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&msg, 100, NULL);
	if(formatResult) {
		CString formattedMsg(msg);
		_msg.Format(_T("%s(0x%x)"), formattedMsg.TrimRight(_T("\r\n")).GetString(), hr);
		LocalFree(msg);
	} else {
		_msg.Format(_T("0x%x"), hr);
	}
	LOG4CXX_ERROR(logger, _T("`") << exp << _T("` failed: ") << _msg.GetString());
}

void CbtswwinDlg::print(const CString& text)
{
	print(text.GetString());
}

void CbtswwinDlg::print(LPCTSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	CString* text = new CString();
	text->FormatV(fmt, args);
	LOG4CXX_INFO(logger, text->GetString());
	if(!PostMessage(WM_USER_PRINT, 0, (LPARAM)text)) {
		delete text;
		CString err;
		err.Format(_T(__FUNCTION__ ": PostMessage(%d) failed. Error=%d\n"), WM_USER_PRINT, GetLastError());
		LOG4CXX_ERROR(logger, err.GetString());
	}
}

afx_msg LRESULT CbtswwinDlg::OnUserPrint(WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<CString> text((CString*)lParam);
	m_StatusText.SetWindowText(text->GetString());
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
	DDX_Control(pDX, IDC_LIST_BLUETOOTH_DEVICE, m_bluetoothDevices);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_StatusText);
	DDX_Check(pDX, IDC_CHECK_SWITCH_BY_LCD_STATE, m_switchByLcdState);
	DDX_Check(pDX, IDC_CHECK_RESTORE_RADIO_STATE, m_restoreRadioState);
}

BEGIN_MESSAGE_MAP(CbtswwinDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_POWERBROADCAST()
	ON_MESSAGE(WM_USER_PRINT, &CbtswwinDlg::OnUserPrint)
	ON_MESSAGE(WM_USER_RADIO_MANAGER_NOTIFY, &CbtswwinDlg::OnUserRadioManagerNotify)
	ON_MESSAGE(WM_USER_CONNECT_DEVICE_RESULT, &CbtswwinDlg::OnUserConnectDeviceResult)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_LOCAL_RADIO_ON, &CbtswwinDlg::OnSwitchRadioUpdateCommandUI)
	ON_UPDATE_COMMAND_UI(ID_LOCAL_RADIO_OFF, &CbtswwinDlg::OnSwitchRadioUpdateCommandUI)
	ON_UPDATE_COMMAND_UI(ID_REMOTE_DEVICE_CONNECT, &CbtswwinDlg::OnConnectDeviceUpdateCommandUI)
	ON_COMMAND_EX(ID_LOCAL_RADIO_ON, &CbtswwinDlg::OnSwitchRadioCommand)
	ON_COMMAND_EX(ID_LOCAL_RADIO_OFF, &CbtswwinDlg::OnSwitchRadioCommand)
	ON_COMMAND(ID_REMOTE_DEVICE_CONNECT, &CbtswwinDlg::OnConnectDeviceCommand)
	ON_COMMAND(ID_EDIT_COPYRADIOLIST, &CbtswwinDlg::OnCopyRadioList)
	ON_COMMAND(ID_EDIT_COPYDEVICELIST, &CbtswwinDlg::OnCopyDeviceList)
	ON_WM_INITMENUPOPUP()
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
#ifdef DEBUG_LIDSWITCH
		strAboutMenu = _T("LCD open/close");
#endif
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
	tsm::Assert::onAssertFailedProc = ::AssertFailedProc;

	m_radioInstances.OnInitCtrl();

	auto hr = createRadioManager();

	if(SUCCEEDED(hr)) {
		m_hPowerNotify = RegisterPowerSettingNotification(m_hWnd, &GUID_LIDSWITCH_STATE_CHANGE, DEVICE_NOTIFY_WINDOW_HANDLE);
		hr = WIN32_EXPECT(m_hPowerNotify);
	}

	m_bluetoothDevices.OnInitCtrl();
	checkBluetoothDevice();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// Create IMedioRadioManager object for Bluetooth device and setup RadioNotifyListener.
HRESULT CbtswwinDlg::createRadioManager()
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
#ifdef DEBUG_LIDSWITCH
		// Send debug message for LCD open/close.
		static BYTE data = 0;
		data = (data == 0) ? 1 : 0;
		POWERBROADCAST_SETTING setting = {
			GUID_LIDSWITCH_STATE_CHANGE,
			sizeof(BYTE),
			data
		};
		SendMessage(WM_POWERBROADCAST, PBT_POWERSETTINGCHANGE, (LPARAM)&setting);
#else
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
#endif
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

// Checks if one or more Radio instances can be switched on/off.
void CbtswwinDlg::OnSwitchRadioUpdateCommandUI(CCmdUI* pCmdUI)
{
	DEVICE_RADIO_STATE state;
	switch(pCmdUI->m_nID) {
	case ID_LOCAL_RADIO_ON:
		state = DRS_RADIO_ON;
		break;
	case ID_LOCAL_RADIO_OFF:
		state = DRS_SW_RADIO_OFF;
		break;
	default:
		DebugPrint(_T(__FUNCTION__ ": Unkown ID %d"), pCmdUI->m_nID);
		return;
	}
	auto enable = FALSE;
	auto data = m_radioInstances.GetSelectedInstance();
	if(data) {
		// A Radio instance selected in the ListCtrl.
		if(state != data->state) { enable = TRUE; }
	} else {
		// All Radio instances checked in the ListCtrl.
		m_radioInstances.For([&](RadioInstanceData& data)
			{
				if(state != data.state) { enable = TRUE; }
				return S_OK;
			});
	}

	pCmdUI->Enable(enable);
}

// Switchs one or more Radio instances on/off
BOOL CbtswwinDlg::OnSwitchRadioCommand(UINT id)
{
	DEVICE_RADIO_STATE state;
	switch(id) {
	case ID_LOCAL_RADIO_ON:
		state = DRS_RADIO_ON;
		break;
	case ID_LOCAL_RADIO_OFF:
		state = DRS_SW_RADIO_OFF;
		break;
	default:
		DebugPrint(_T(__FUNCTION__ ": Unkown ID %d"), id);
		return FALSE;
	}

	auto data = m_radioInstances.GetSelectedInstance();
	if(data) {
		// Switch radio instance selected in the ListCtrl.
		HR_EXPECT_OK(setRadioState(*data, state));
	} else {
		// Switch all radio instances checked in the ListCtrl.
		HR_EXPECT_OK(setRadioState(state));
	}

	return TRUE;
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

// Sets state of all Radio instances checked in the ListCtrl.
HRESULT CbtswwinDlg::setRadioState(DEVICE_RADIO_STATE state, bool restore /*= false*/)
{
	return m_radioInstances.For([=](RadioInstanceData& data)
		{
			return setRadioState(data, state, restore);
		}
	);
}

// Sets state of specified Radio instance.
HRESULT CbtswwinDlg::setRadioState(RadioInstanceData& data, DEVICE_RADIO_STATE state, bool restore /*= false*/)
{
	auto newState = restore ? data.savedState : state;
	HR_ASSERT_OK(data.radioInstance->GetRadioState(&data.state));
	return (data.state != newState) ?
		HR_EXPECT_OK(data.radioInstance->SetRadioState(state, 1)) :
		S_FALSE;
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
			// Retrieve FriendlyName, Signature and RadioState from IRadioInstance object.
			type = _T("InstanceAdd");
			BSTR friendlyName, id;
			message->radioInstance->GetFriendlyName(1033, &friendlyName);
			HR_ASSERT_OK(message->radioInstance->GetInstanceSignature(&id));
			HR_ASSERT_OK(message->radioInstance->GetRadioState(&state));
			RadioInstanceData data(
				{
					message->radioInstance, id, friendlyName,
					message->radioInstance->IsMultiComm(),
					message->radioInstance->IsAssociatingDevice(),
					state, state
				});
			m_radioInstances.Add(data);
			name.Format(_T("%s:%s"), data.id.GetString(), data.name.GetString());

			// Start polling timer when first instance is added.
			if(m_radioInstances.GetItemCount() == 1) { SetTimer(PollingTimerId, 1000, NULL); }
		}
		break;
	case RadioNotifyListener::Message::Type::InstanceRemove:
		// RadioNotifyListener::OnInstanceRemove(BSTR bstrRadioInstanceId)
		type = _T("InstanceRemove");
		name = message->radioInstanceId;
		m_radioInstances.Remove(name);

		// Stop polling timer when last instance is removed.
		if(m_radioInstances.GetItemCount() == 0) { KillTimer(PollingTimerId); }
		break;
	case RadioNotifyListener::Message::Type::InstanceRadioChange:
		// RadioNotifyListener::OnInstanceRadioChange(BSTR bstrRadioInstanceId, DEVICE_RADIO_STATE radioState)
		type = _T("InstanceRadioChange");
		name = message->radioInstanceId;
		state = message->radioState;
		m_radioInstances.StateChange(name, state);
		break;
	}
	UpdateData(FALSE);

	print(_T("%s: %s, %s"), type.GetString(), name.GetString(), ValueToString(states, state).GetString());
	return 0;
}

HRESULT CbtswwinDlg::checkRadioState()
{
	auto hrCheckRadioState = m_radioInstances.For([this](RadioInstanceData& data)
		{
			using um = CRadioInstanceList::UpdateMask;
			auto updateMask = um::None;
			CStringArray changes;
			auto isMultiComm = data.radioInstance->IsMultiComm();
			if(data.isMultiComm != isMultiComm) {
				CString str;
				str.Format(_T("IsMultiComm %s"), boolToString(isMultiComm));
				changes.Add(str);
				data.isMultiComm = isMultiComm;
				updateMask |= um::IsMultiComm;
			}
			auto isAssocDev = data.radioInstance->IsAssociatingDevice();
			if(data.isAssociatingDevice != isAssocDev) {
				CString str;
				str.Format(_T("IsAssociatingDevice %s"), boolToString(isAssocDev));
				changes.Add(str);
				data.isAssociatingDevice = isAssocDev;
				updateMask |= um::IsAssocDev;
			}
			if(updateMask != um::None) {
				print(_T("%s: %s"), data.id.GetString(), join(changes).GetString());
				HR_ASSERT_OK(m_radioInstances.Update(data, updateMask));
			}
			return S_OK;
		},
		false
	);
	return HR_EXPECT_OK(hrCheckRadioState);
}

HRESULT CbtswwinDlg::checkBluetoothDevice()
{
	// Find Bluetooth devices and add them to newList.
	BLUETOOTH_DEVICE_SEARCH_PARAMS params{sizeof(params)};
	params.fReturnAuthenticated = TRUE;
	params.fReturnRemembered = TRUE;
	params.fReturnConnected = TRUE;
	params.fReturnUnknown = TRUE;
	BLUETOOTH_DEVICE_INFO deviceInfo{sizeof(deviceInfo)};
	auto hFind = BluetoothFindFirstDevice(&params, &deviceInfo);
	if(!hFind) { return S_FALSE; }

	CBluetoothDeviceList::ListData newList;
	do {
		newList[CBluetoothDeviceList::getListKey(deviceInfo)] = deviceInfo;
	} while(BluetoothFindNextDevice(hFind, &deviceInfo));
	WIN32_EXPECT(BluetoothFindDeviceClose(hFind));

	auto& currentList = m_bluetoothDevices.getDeviceInfoList();

	// Check if any device is added or changed.
	for(auto& x : newList) {
		const auto it = currentList.find(x.first);
		if(it == currentList.end()) {
			// The device is added.
			CString deviceName(x.second.szName);
			print(_T("DeviceAdd %s"), deviceName.GetString());
			m_bluetoothDevices.Add(x.second);
		} else {
			// The device already exists.
			CStringArray changes;
			if(x.second.fConnected != it->second.fConnected) {
				changes.Add(x.second.fConnected ? _T("Connected") : _T("Disconnected"));
			}
			if(x.second.fAuthenticated != it->second.fAuthenticated) {
				changes.Add(x.second.fAuthenticated ? _T("Authenticated") : _T("Unauthenticated"));
			}
			if(x.second.fRemembered != it->second.fRemembered) {
				changes.Add(x.second.fRemembered ? _T("Remembered") : _T("Unremembered"));
			}
			if(!changes.IsEmpty()) {
				// State of the device is changed.
				CString deviceName(x.second.szName);
				print(_T("DeviceStateChange %s: %s"), deviceName.GetString(), join(changes).GetString());
				m_bluetoothDevices.StateChange(x.second);
			}
		}
	}

	// Check if any device is removed.
	// While connecting thread is running, any device can not be removed.
	if(!m_connectDeviceThread) {
		bool removed;
		do {
			removed = false;
			for(auto& x : currentList) {
				const auto it = newList.find(x.first);
				if(it == newList.end()) {
					// The device is removed.
					CString deviceName(x.second.szName);
					print(_T("DeviceRemove %s"), deviceName.GetString());
					m_bluetoothDevices.Remove(x.second);
					// Start from the beginning of currentList
					// because it has been changed by CBluetoothDeviceList::Remove().
					removed = true;
					break;
				}
			}
		} while(removed);
	}

	return S_OK;
}

// Checks if the device can be connected.
void CbtswwinDlg::OnConnectDeviceUpdateCommandUI(CCmdUI* pCmdUI)
{
	BOOL enable = TRUE;

	// While connecting thread is running, new connection can be started.
	if(m_connectDeviceThread) { enable = FALSE; }

	// Check if the device is not connected yet.
	auto deviceInfo = m_bluetoothDevices.GetSelectedDevice();
	if(!deviceInfo || (deviceInfo->fConnected)) { enable = FALSE; }

	// Check if at least one radio instance is on.
	auto onCount = 0;
	m_radioInstances.For([this, &onCount](RadioInstanceData& data)
		{
			if(data.state == DRS_RADIO_ON) { onCount++; }
			return S_OK;
		}, false);
	if(onCount == 0) { enable = FALSE; }

	pCmdUI->Enable(enable);
}

// Connects the device selected by the user.
void CbtswwinDlg::OnConnectDeviceCommand()
{
	BeginWaitCursor();
	m_connectDeviceThread = std::make_unique<std::thread>([this]
		{
			HANDLE hRadio = NULL;		// Search for all local radios.
			DWORD serviceCount = 0;
			auto deviceInfo = m_bluetoothDevices.GetSelectedDevice();
			auto enumError = BluetoothEnumerateInstalledServices(hRadio, deviceInfo, &serviceCount, NULL);
			HR_EXPECT(enumError == ERROR_MORE_DATA, HRESULT_FROM_WIN32(enumError));
			CString deviceName(deviceInfo->szName);
			if(serviceCount) {
				print(_T("Connecting to %s(%d services) ..."), deviceName.GetString(), serviceCount);
				auto serviceGuids = std::make_unique<GUID[]>(serviceCount);
				HR_EXPECT_OK(HRESULT_FROM_WIN32(BluetoothEnumerateInstalledServices(hRadio, deviceInfo, &serviceCount, serviceGuids.get())));
				for(DWORD i = 0; i < serviceCount; i++) {
					const auto& guid = serviceGuids.get()[i];
					//WCHAR strGuid[40];
					//HR_EXPECT(0 < StringFromGUID2(guid, strGuid, ARRAYSIZE(strGuid)), HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
					//print(_T("Setting service state of %s"), strGuid);
					HR_EXPECT_OK(HRESULT_FROM_WIN32(BluetoothSetServiceState(hRadio, deviceInfo, &guid, BLUETOOTH_SERVICE_DISABLE)));
					HR_EXPECT_OK(HRESULT_FROM_WIN32(BluetoothSetServiceState(hRadio, deviceInfo, &guid, BLUETOOTH_SERVICE_ENABLE)));
				}
			} else {
				print(_T("No installed service to connect for %s"), deviceName.GetString());
			}

			PostMessage(WM_USER_CONNECT_DEVICE_RESULT, serviceCount, (LPARAM)deviceInfo);
		});
}

// Handles WM_USER_CONNECT_DEVICE_RESULT message.
// Checks whether the device is connected successfully.
LRESULT CbtswwinDlg::OnUserConnectDeviceResult(WPARAM wParam, LPARAM lParam)
{
	auto serviceCount = (DWORD)wParam;
	if(serviceCount) {
		auto pDeviceInfo = (BLUETOOTH_DEVICE_INFO*)lParam;
		HR_EXPECT_OK(HRESULT_FROM_WIN32(BluetoothGetDeviceInfo(nullptr, pDeviceInfo)));
		print(_T("%s to connect"), pDeviceInfo->fConnected ? _T("Succeeded") : _T("Failed"));
	}

	m_connectDeviceThread->join();
	m_connectDeviceThread.reset();
	EndWaitCursor();
	return LRESULT(0);
}

void CbtswwinDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent) {
	case PollingTimerId:
		checkRadioState();
		checkBluetoothDevice();
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void DebugPrint(LPCTSTR fmt, ...)
{
	if(IsDebuggerPresent()) {
		va_list args;
		va_start(args, fmt);
		CString msg;
		msg.FormatV(fmt, args);
		va_end(args);
		OutputDebugString(msg.GetString());
		OutputDebugString(_T("\n"));
	}
}

// Work-around for ON_UPDATE_COMMAND_UI
// See https://learn.microsoft.com/en-us/troubleshoot/developer/visualstudio/cpp/libraries/cannot-change-state-menu-item
void CbtswwinDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    // Make sure this is actually a menu. When clicking the program icon
    // in the window title bar this function will trigger and pPopupMenu 
    // will NOT be a menu.
    if (!IsMenu(pPopupMenu->m_hMenu))
		return;
        
    ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.

    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
    state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd))!= NULL)
    {
        CWnd* pParent = this;
        // Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
        (hParentMenu = ::GetMenu(pParent->m_hWnd))!= NULL)
        {
            int nIndexMax = ::GetMenuItemCount(hParentMenu);
            for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
            {
                if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
            {
                // When popup is found, m_pParentMenu is containing menu.
                state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                break;
            }
            }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
    state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
        continue; // Menu separator or invalid cmd - ignore it.

        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
            // Possibly a popup menu, route to first item of that popup.
            state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
            if (state.m_pSubMenu == NULL ||
            (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
            state.m_nID == (UINT)-1)
            {
            continue; // First item of popup can't be routed to.
            }
            state.DoUpdate(this, TRUE); // Popups are never auto disabled.
        }
        else
        {
            // Normal menu item.
            // Auto enable/disable if frame window has m_bAutoMenuEnable
            // set and command is _not_ a system command.
            state.m_pSubMenu = NULL;
            state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
            state.m_nIndex -= (state.m_nIndexMax - nCount);
            while (state.m_nIndex < nCount &&
            pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
            {
                state.m_nIndex++;
            }
        }
        state.m_nIndexMax = nCount;
    }
}


BOOL CbtswwinDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN) {
		switch(pMsg->wParam) {
		case VK_RETURN:
		case VK_ESCAPE:
			return FALSE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
