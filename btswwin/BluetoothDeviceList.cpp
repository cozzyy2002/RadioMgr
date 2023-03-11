#include "pch.h"
#include "BluetoothDeviceList.h"
#include "resource.h"
#include "../Common/Assert.h"

#define COLUMN_TITLE_ITEM(c, s, l) {int(CBluetoothDeviceList::Column::##c), _T(s), l * 8}
static CBluetoothDeviceList::ColumnTitle columns[] = {
    COLUMN_TITLE_ITEM(Address, "Address", 16),
    COLUMN_TITLE_ITEM(Name, "Name", 16),
	COLUMN_TITLE_ITEM(ClassOfDevice, "Class of Device", 20),
	COLUMN_TITLE_ITEM(Connected, "Connected", 8),
    COLUMN_TITLE_ITEM(Authenticated, "Auth", 8),
    COLUMN_TITLE_ITEM(Remembered, "Remembered", 8),
};

HRESULT CBluetoothDeviceList::OnInitCtrl()
{
    auto exStyle = LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT;
    SetExtendedStyle(exStyle | GetExtendedStyle());

    // Setup colums
    setupColumns(columns);

    // Setup image list for Bluetooth on/off icon.
    static const UINT bitmaps[] = {IDB_BITMAP_DEVICE_CONNECTED, IDB_BITMAP_DEVICE_DISCONNECTED};
    setupImageList(bitmaps);

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
    if(mask & UpdateMask::ConnectIcon) { setItemImage(nItem, info.fConnected ? IDB_BITMAP_DEVICE_CONNECTED : IDB_BITMAP_DEVICE_DISCONNECTED); }
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

CString addressToString(const BLUETOOTH_ADDRESS& address)
{
    CString str;
    auto& adr(address.rgBytes);
    str.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"), adr[5], adr[4], adr[3], adr[2], adr[1], adr[0]);
    return str;
}
