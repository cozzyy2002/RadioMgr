#include "pch.h"
#include "BluetoothDeviceList.h"
#include "resource.h"
#include "../Common/Assert.h"

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

    return E_NOTIMPL;
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
    auto address = addressToString(info.Address);
    auto nItem = findItem(address);
    HR_ASSERT(-1 < nItem, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

    if(mask & UpdateMask::Name) { SetItemText(nItem, int(Column::Name), info.szName); }
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

// Returns image ID suitable for ClassofDevice value.
/*static*/ UINT getDeviceImageId(ULONG ulClassofDevice)
{
    auto major = GET_COD_MAJOR(ulClassofDevice);
    auto minor = GET_COD_MINOR(ulClassofDevice);

    UINT id = 0;
    for(auto& x : deviceImageIdList) {
        if((x.major == major) && (x.minor == minor)) {
            return x.imageId;
        }
    }

    return IDB_BITMAP_UNKNOWN_DEVICE;
}

CString addressToString(const BLUETOOTH_ADDRESS& address)
{
    CString str;
    auto& adr(address.rgBytes);
    str.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"), adr[5], adr[4], adr[3], adr[2], adr[1], adr[0]);
    return str;
}
