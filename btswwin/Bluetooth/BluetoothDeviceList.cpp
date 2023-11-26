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
        SetItemText(nItem, int(Column::Name), getDeviceName(info).GetString());
    }
	if(mask & UpdateMask::ClassofDevice) {
		CString majorDeviceClass, minorDeviceClass;
		ClassOfDeviceToString(info.ulClassofDevice, nullptr, &majorDeviceClass, &minorDeviceClass);
		CString text;
		text.Format(_T("%s:%s"), majorDeviceClass.GetString(), minorDeviceClass.GetString());
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

CString getDeviceName(const BLUETOOTH_DEVICE_INFO& info)
{
	if(info.szName[0] != L'\0') {
		return info.szName;
	} else {
		return addressToString(info.Address);
	}
}

UINT CBluetoothDeviceList::getContextMenuId() const
{
    return IDR_MENU_DEVICE_LIST;
}


// TODO: Implement showing Minor Device Class.
//       Currently implemented for Computer, Phone(not tested) and Audio/Video device.

// Struct defines Major and Minor Device Class.
struct MajorMinorDeviceClass {
	LPCTSTR major;			// Major Device Class.
	size_t minorSize;		// Array size of Minor Device Classes.
	const LPCTSTR* pMinor;	// Minor Device Classes array.

	// Function that returns Minor Device Class corresponding to ULONGLONG value.
	using MinorDeviceClassFunc = CString(*)(const MajorMinorDeviceClass&, ULONGLONG);
	MinorDeviceClassFunc minorDeviceClassFunc;
};

static CString DefaultMinorDeviceCalssFunc(const MajorMinorDeviceClass&, ULONGLONG);

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

static const MajorMinorDeviceClass MajorDeviceClasses[] = {
	{_T("Miscellaneous")},
	{_T("Computer"), ARRAYSIZE(ComputerMinorDeviceClasses), ComputerMinorDeviceClasses},
	{_T("Phone"), ARRAYSIZE(PhoneMinorDeviceClasses), PhoneMinorDeviceClasses},
	{_T("LAN/Network Access point")},
	{_T("Audio/Video"), ARRAYSIZE(AudioVideoMinorDeviceClasses), AudioVideoMinorDeviceClasses},
	{_T("Peripheral")},
	{_T("Imaging")},
	{_T("Wearable")},
	{_T("Toy")},
	{_T("Health")},
	{_T("Uncategorized: device code not specified")},
};

// Returns string representing MajorServiceClass, MajorDeviceClass and MinorDeviceClass for Class of Device value.
// See https://btprodspecificationrefs.blob.core.windows.net/assigned-numbers/Assigned%20Number%20Types/Assigned%20Numbers.pdf
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
		if(!func) { func = DefaultMinorDeviceCalssFunc; }
		*pMinorDeviceClass = func(deviceClass, minorDeviceClass);
	}
}

// Returns Minor Device Class corresponding to the value.
// The value is used as index of array that consits of Minor Devie Class strings.
CString DefaultMinorDeviceCalssFunc(const MajorMinorDeviceClass& classes, ULONGLONG value)
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
