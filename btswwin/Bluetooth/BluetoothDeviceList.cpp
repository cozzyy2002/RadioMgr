#include "pch.h"
#include "BluetoothDeviceList.h"
#include "resource.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CBluetoothDeviceList")));

#define COLUMN_TITLE_ITEM(c, s, l) {int(CBluetoothDeviceList::Column::##c), _T(s), l * 8}
static const CBluetoothDeviceList::ColumnTitle columns[] = {
    COLUMN_TITLE_ITEM(Address, "Address", 16),
    COLUMN_TITLE_ITEM(Name, "Name", 16),
	COLUMN_TITLE_ITEM(ClassOfDevice, "Class of Device", 20),
	COLUMN_TITLE_ITEM(Connected, "Connected", 8),
    COLUMN_TITLE_ITEM(Authenticated, "Auth", 8),
    COLUMN_TITLE_ITEM(Remembered, "Remembered", 8),
};

// Array of Bitmap ID to be shown in each item.
static const UINT bitmaps[] = {
    IDB_BITMAP_UNKNOWN_DEVICE,
    IDB_BITMAP_DEVICE_HEADPHONE,
    IDB_BITMAP_WEARABLE_HEADSET,
    IDB_BITMAP_LAPTOP_COMPUTER,
    IDB_BITMAP_DEVICE_CONNECTED_OVERLAY     // Overlay image to display connected device.
};

// Structure to map Major/Minor code of ClassofDevice value and image ID.
struct DeviceImageId {
    BYTE major;
    BYTE minor;
    UINT imageId;
};

// Image IDs for each Major/Minor code of the device.
static const DeviceImageId deviceImageIdList[] = {
    {COD_MAJOR_AUDIO, COD_AUDIO_MINOR_HEADSET, IDB_BITMAP_WEARABLE_HEADSET},
    {COD_MAJOR_AUDIO, COD_AUDIO_MINOR_HEADPHONES, IDB_BITMAP_DEVICE_HEADPHONE},
    {COD_MAJOR_COMPUTER, COD_COMPUTER_MINOR_LAPTOP, IDB_BITMAP_LAPTOP_COMPUTER},
};

static UINT getDeviceImageId(ULONG ulClassofDevice);
static void ClassOfDeviceToString(ULONG value, CString* pServiceClasses, CString* pMajorDeviceClass, CString* pMinorDeviceClass);


HRESULT CBluetoothDeviceList::OnInitCtrl()
{
    auto exStyle = LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT;
    SetExtendedStyle(exStyle | GetExtendedStyle());

    // Setup colums
    setupColumns(columns);

    // Setup image list for Bluetooth on/off icon.
    setupImageList(bitmaps);
    setOverlayImage(IDB_BITMAP_DEVICE_CONNECTED_OVERLAY, 1);

    return S_OK;
}

HRESULT CBluetoothDeviceList::Add(const BLUETOOTH_DEVICE_INFO& info)
{
    auto& pair = m_infos.insert({getListKey(info), info});

    auto nItem = addItem(addressToString(info.Address));
    SetItemData(nItem, (DWORD_PTR)&pair.first->second);
    return Update(info, UpdateMask::All);
}

HRESULT CBluetoothDeviceList::Remove(const BLUETOOTH_DEVICE_INFO& info)
{
    auto it = m_infos.find(getListKey(info));
    HR_ASSERT(it != m_infos.end(), HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

    m_infos.erase(it);
    removeItem(addressToString(info.Address));

    return S_OK;
}

HRESULT CBluetoothDeviceList::StateChange(const BLUETOOTH_DEVICE_INFO& info)
{
    auto it = m_infos.find(getListKey(info));
    HR_ASSERT(it != m_infos.end(), HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

    it->second = info;
    return Update(info, UpdateMask::State);
}

HRESULT CBluetoothDeviceList::Update(const BLUETOOTH_DEVICE_INFO& info, UpdateMask mask)
{
    auto nItem = findItem(info);
    HR_ASSERT(-1 < nItem, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

    if(mask & UpdateMask::Name) {
        SetItemText(nItem, int(Column::Name), CString(info.szName).GetString());
    }
	if(mask & UpdateMask::ClassofDevice) {
		CString majorDeviceClass, minorDeviceClass;
		ClassOfDeviceToString(info.ulClassofDevice, nullptr, &majorDeviceClass, &minorDeviceClass);
		CString text;
		if(!minorDeviceClass.IsEmpty()) {
			text.Format(_T("%s:%s"), majorDeviceClass.GetString(), minorDeviceClass.GetString());
		} else {
			text = majorDeviceClass;
		}
		SetItemText(nItem, int(Column::ClassOfDevice), text);
	}
    if(mask & UpdateMask::ConnectIcon) {
        setItemImage(nItem, getDeviceImageId(info.ulClassofDevice), info.fConnected ? 1 : 0);
    }
    if(mask & UpdateMask::ConnectText) { SetItemText(nItem, int(Column::Connected), boolToString(info.fConnected)); }
    if(mask & UpdateMask::Authenticated) { SetItemText(nItem, int(Column::Authenticated), boolToString(info.fAuthenticated)); }
    if(mask & UpdateMask::Remembered) { SetItemText(nItem, int(Column::Remembered), boolToString(info.fRemembered)); }

    return S_OK;
}


const BLUETOOTH_DEVICE_INFO* CBluetoothDeviceList::GetSelectedDevice()
{
    BLUETOOTH_DEVICE_INFO* ret = nullptr;
    auto pos = GetFirstSelectedItemPosition();
    if(pos) {
        auto i = GetNextSelectedItem(pos);
        ret = (BLUETOOTH_DEVICE_INFO*)GetItemData(i);
    }
    return ret;
}

int CBluetoothDeviceList::findItem(const BLUETOOTH_DEVICE_INFO& info)
{
	auto address = addressToString(info.Address);
	return CItemList::findItem(address);
}

// Returns image ID suitable for ClassofDevice value.
/*static*/ UINT getDeviceImageId(ULONG ulClassofDevice)
{
    auto major = GET_COD_MAJOR(ulClassofDevice);
    auto minor = GET_COD_MINOR(ulClassofDevice);

    for(auto& x : deviceImageIdList) {
        if((x.major == major) && (x.minor == minor)) {
            return x.imageId;
        }
    }

    DebugPrint(_T(__FUNCTION__ ": Unknown Class of Device 0x%06x, Major=0x%02x, Minor0x%02x"), ulClassofDevice, major, minor);

    return IDB_BITMAP_UNKNOWN_DEVICE;
}

CString addressToString(const BLUETOOTH_ADDRESS& address)
{
    CString str;
    auto& adr(address.rgBytes);
    str.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"), adr[5], adr[4], adr[3], adr[2], adr[1], adr[0]);
    return str;
}

// Returns Bluetooth device name.
// If the name is empty, returns its address as string.
CString getDeviceNameOrAddress(const BLUETOOTH_DEVICE_INFO& info)
{
	if(info.szName[0] != L'\0') {
		return CString(info.szName);
	} else {
		return addressToString(info.Address);
	}
}

UINT CBluetoothDeviceList::getContextMenuId() const
{
    return IDR_MENU_DEVICE_LIST;
}


// Struct defines Major and Minor Device Class.
struct MajorMinorDeviceClass {
	LPCTSTR major;			// Major Device Class.
	size_t minorSize;		// Array size of Minor Device Classes.
	const LPCTSTR* pMinor;	// Minor Device Classes array.

	// Function that returns Minor Device Class corresponding to ULONGLONG value.
	using MinorDeviceClassFunc = CString(*)(const MajorMinorDeviceClass&, ULONGLONG);
	MinorDeviceClassFunc minorDeviceClassFunc;
};

static CString DefaultMinorDeviceClassFunc(const MajorMinorDeviceClass&, ULONGLONG);

// Returns empty string.
static CString MiscellaneousMinorDeviceClassFunc(const MajorMinorDeviceClass&, ULONGLONG)
{
	return _T("");
}

static const LPCTSTR MajorServiceClasses[] = {
	/*Bit*/
	/* 13*/ _T("Limited Discoverable Mode"),
	/* 14*/ _T("LE audio"),
	/* 15*/ _T("Reserved for future use"),
	/* 16*/ _T("Positioning(Location identification)"),
	/* 17*/ _T("Networking"),
	/* 18*/ _T("Rendering"),
	/* 19*/ _T("Capturing"),
	/* 20*/ _T("Object Transfer"),
	/* 21*/ _T("Audio"),
	/* 22*/ _T("Telephony"),
	/* 23*/ _T("Information")
};

static const LPCTSTR ComputerMinorDeviceClasses[] = {
	_T("Uncategorized, code for devie not assigned"),
	_T("Desktop workstation"),
	_T("Server-class computer"),
	_T("Laptop"),
	_T("Handheld PC/PDA(clamshell)"),
	_T("Palm-size PC/PDA"),
	_T("Wearable computer(watch size)"),
	_T("Tablet"),
};

static const LPCTSTR PhoneMinorDeviceClasses[] = {
	_T("Uncategorized, code for device not assigned"),
	_T("Cellular"),
	_T("Cordless"),
	_T("Smartphone"),
	_T("Wired modem or void gateway"),
	_T("Common ISDN access"),
};

static CString LanNetworkMinorDeviceClassFunc(const MajorMinorDeviceClass&, ULONGLONG value)
{
	static const LPCTSTR MinorDeviceClasses[] = {
		_T("Fully available"),
		_T("1% to 17% utilized"),
		_T("17% to 33% utilized"),
		_T("33% to 50% utilized"),
		_T("50% to 67% utilized"),
		_T("67% to 83% utilized"),
		_T("83% to 99% utilized"),
		_T("No service available"),
	};
	// Bit 7 - 5 of value are used as index.
	auto index = (value >> 3) & 0x07;
	return MinorDeviceClasses[index];
}

static const LPCTSTR AudioVideoMinorDeviceClasses[] = {
	_T("Uncategorized, code not assigned"),
	_T("Wearable Headset Device"),
	_T("Hands-free Device"),
	_T("(Reserved)"),
	_T("Microphone"),
	_T("Loudspeaker"),
	_T("Headphones"),
	_T("Portable Audio"),
	_T("Car Audio"),
	_T("Set-top box"),
	_T("HiFi Audio Device"),
	_T("VCR"),
	_T("Video Camera"),
	_T("Camcorder"),
	_T("Video Monitor"),
	_T("Video Display and Loudspeaker"),
	_T("Video Conferencing"),
	_T("(Reserved)"),
	_T("Gaming/Toy"),
};

static CString PeripheralMinorDeviceClassFunc(const MajorMinorDeviceClass& classes, ULONGLONG value)
{
	// Assigned to bit 7 - 6
	static const LPCTSTR UpperBits[] = {
		_T("Uncategorized"),
		_T("Keyboard"),
		_T("Pointing Device"),
		_T("Combo Keyboard/Pointing Device"),
	};

	// Assigned to bit 5 - 2
	static const LPCTSTR LowerBits[] = {
		_T("Uncategorized"),
		_T("Joystick"),
		_T("Gamepad"),
		_T("Remote Control"),
		_T("Sensing Device"),
		_T("Digitizer Tablet"),
		_T("Card Reader"),
		_T("Digital Pen"),
		_T("Handheld Scanner"),
		_T("Handheld Gestural Input Device"),
	};

	LPCTSTR upper, lower;
	// Note: value is passed after shifted 2 bits to the right.
	auto index = (value >> 4) & 3;
	if(index < ARRAYSIZE(UpperBits)) {
		upper = UpperBits[index];
	} else {
		return DefaultMinorDeviceClassFunc(classes, value);
	}
	index = value & 0x0f;
	if(index < ARRAYSIZE(LowerBits)) {
		lower = LowerBits[index];
	} else {
		return DefaultMinorDeviceClassFunc(classes, value);
	}
	CString ret;
	ret.Format(_T("%s/%s"), upper, lower);
	return ret;
}

static CString ImagingMinorDeviceClassFunc(const MajorMinorDeviceClass& classes, ULONGLONG value)
{
	// MinorDeviceClass bitmap and name for Imaging MajorDeviceClass.
	static const struct {
		ULONGLONG value;
		LPCTSTR name;
	} MinorDeviceClasses[] = {
		{ 0b000100, _T("Display") },
		{ 0b001000, _T("Camera") },
		{ 0b010000, _T("Scanner") },
		{ 0b100000, _T("Printer") },
	};
	for(auto& x : MinorDeviceClasses) {
		if(value & x.value) { return x.name; }
	}
	return DefaultMinorDeviceClassFunc(classes, value);
}
static const LPCTSTR WearableMinorDeviceClasses[] = {
	_T("N/A"),			// value 0 is not used.
	_T("Wristwatch"),
	_T("Pager"),
	_T("Jacket"),
	_T("Helmet"),
	_T("Glasses"),
	_T("Pin"),
};

static const LPCTSTR ToyMinorDeviceClasses[] = {
	_T("N/A"),			// value 0 is not used.
	_T("Robot"),
	_T("Hehicle"),
	_T("Doll/Action Figure"),
	_T("Controller"),
	_T("Game"),
};

static const LPCTSTR HealthMinorDeviceClasses[] = {
	_T("Undefined"),
	_T("Blood Pressure Monitor"),
	_T("Thermometer"),
	_T("Weighing Scale"),
	_T("Glucose Meter"),
	_T("Pulse Oximeter"),
	_T("Heart/Pulse Rate Monitor"),
	_T("Health Data Display"),
	_T("Step Counter"),
	_T("Body Composition Analyzer"),
	_T("Peak Flow Monitor"),
	_T("Medication Monitor"),
	_T("Knee Prosthesis"),
	_T("Ankle Prosthesis"),
	_T("Generic Health Manager"),
	_T("Personal Mobility Device"),
};

static const MajorMinorDeviceClass MajorDeviceClasses[] = {
	{_T("Miscellaneous"), 0, nullptr, MiscellaneousMinorDeviceClassFunc},
	{_T("Computer"), ARRAYSIZE(ComputerMinorDeviceClasses), ComputerMinorDeviceClasses},
	{_T("Phone"), ARRAYSIZE(PhoneMinorDeviceClasses), PhoneMinorDeviceClasses},
	{_T("LAN/Network Access point"), 0, nullptr, LanNetworkMinorDeviceClassFunc},
	{_T("Audio/Video"), ARRAYSIZE(AudioVideoMinorDeviceClasses), AudioVideoMinorDeviceClasses},
	{_T("Peripheral"), 0, nullptr, PeripheralMinorDeviceClassFunc},
	{_T("Imaging"), 0, nullptr, ImagingMinorDeviceClassFunc},
	{_T("Wearable"), ARRAYSIZE(WearableMinorDeviceClasses), WearableMinorDeviceClasses},
	{_T("Toy"), ARRAYSIZE(ToyMinorDeviceClasses), ToyMinorDeviceClasses},
	{_T("Health"), ARRAYSIZE(HealthMinorDeviceClasses), HealthMinorDeviceClasses},
	{_T("Uncategorized: device code not specified")},
};

// Returns string representing MajorServiceClass, MajorDeviceClass and MinorDeviceClass for Class of Device value.
// See https://www.bluetooth.com/specifications/an/
void ClassOfDeviceToString(ULONG value, CString* pServiceClasses, CString* pMajorDeviceClass, CString* pMinorDeviceClass)
{
	auto majorServiceClass = GET_COD_SERVICE(value);
	auto majorDeviceClass = GET_COD_MAJOR(value);
	auto minorDeviceClass = GET_COD_MINOR(value);

	if(pServiceClasses) {
		CStringArray serviceClasses;
		auto var = majorServiceClass;
		for(auto str : MajorServiceClasses) {
			if(var & 1) {
				serviceClasses.Add(str);
			}
			var >>= 1;
			if(var == 0) { break; /* No more bit is set.*/ }
		}
		*pServiceClasses = join(serviceClasses, _T("|"));
	}

	auto index = (majorDeviceClass < ARRAYSIZE(MajorDeviceClasses)) ? majorDeviceClass : (ARRAYSIZE(MajorDeviceClasses) - 1);
	auto& deviceClass = MajorDeviceClasses[index];

	if(pMajorDeviceClass) {
		*pMajorDeviceClass = deviceClass.major;
	}

	if(pMinorDeviceClass) {
		auto func = deviceClass.minorDeviceClassFunc;
		if(!func) { func = DefaultMinorDeviceClassFunc; }
		*pMinorDeviceClass = func(deviceClass, minorDeviceClass);
	}
}

// Returns Minor Device Class corresponding to the value.
// The value is used as index of array that consits of Minor Devie Class strings.
CString DefaultMinorDeviceClassFunc(const MajorMinorDeviceClass& classes, ULONGLONG value)
{
	LPCTSTR minor = _T("Unknown");
	if(classes.minorSize && classes.pMinor) {
		if(value < classes.minorSize) {
			minor = classes.pMinor[value];
		} else {
			LOG4CXX_WARN(logger, _T(__FUNCTION__ "(") << classes.major << _T(", ") << value << _T("): Out of range"));
			minor = _T("Out of range");
		}
	} else {
		LOG4CXX_WARN(logger, _T(__FUNCTION__ "(") << classes.major << _T(", ") << value << _T("): Unknown"));
	}

	return minor;
}
