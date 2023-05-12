#pragma once
#include "ItemList.h"

#include <bluetoothapis.h>
#include <map>

// List control class that shows Bluetooth devices retrieved by Bluetooth API.
class CBluetoothDeviceList : public CItemList
{
public:
	// Column index(Sub item index)
	enum class Column {
		Address = 0,
		Name,
		ClassOfDevice,
		Connected,
		Authenticated,
		Remembered,
	};

	HRESULT OnInitCtrl();
	HRESULT Add(const BLUETOOTH_DEVICE_INFO&, BOOL select);
	HRESULT Remove(const BLUETOOTH_DEVICE_INFO&);
	HRESULT StateChange(const BLUETOOTH_DEVICE_INFO&);
	const BLUETOOTH_DEVICE_INFO* GetSelectedDevice();

	//HRESULT For(std::function<HRESULT(const BLUETOOTH_DEVICE_INFO&)> data);

	enum class UpdateMask {
		None = 0,
		Address = 1,
		Name = 2,
		ClassofDevice = 4,
		ConnectIcon = 0x10,
		ConnectText = 0x20,
		Connect = ConnectIcon | ConnectText,
		Authenticated = 0x40,
		Remembered = 0x80,
		State = Connect | Authenticated | Remembered,
		All = 0xff
	};
	HRESULT Update(const BLUETOOTH_DEVICE_INFO& info, UpdateMask mask);

	// Type of Bluetooth device info list.
	// Key is BLUETOOTH_ADDRESS_STRUCT::ullLong contained in BLUETOOTH_DEVICE_INFO.
	using ListData = std::map<ULONGLONG, BLUETOOTH_DEVICE_INFO>;

	int findItem(const BLUETOOTH_DEVICE_INFO&);
	const auto& getDeviceInfoList() { return m_infos; }
	static auto getListKey(const BLUETOOTH_DEVICE_INFO& info) { return info.Address.ullLong; }

protected:
	// Bluetooth device info added.
	ListData m_infos;

	virtual UINT getContextMenuId() const override;
};

CString addressToString(const BLUETOOTH_ADDRESS&);

#pragma region Operator functions for CBluetoothDeviceList::UpdateMask to be used as flag.
inline bool operator&(CBluetoothDeviceList::UpdateMask a, CBluetoothDeviceList::UpdateMask b)
{
	return bool(int(a) & int(b));
}

inline CBluetoothDeviceList::UpdateMask operator|(CBluetoothDeviceList::UpdateMask a, CBluetoothDeviceList::UpdateMask b)
{
	return CBluetoothDeviceList::UpdateMask(int(a) | int(b));
}

inline CBluetoothDeviceList::UpdateMask& operator|=(CBluetoothDeviceList::UpdateMask& _this, CBluetoothDeviceList::UpdateMask _that)
{
	_this = _this | _that;
	return _this;
}
#pragma endregion
