#include <Windows.h>
#include <sal.h>
#include <bthsdpdef.h>
#include <bluetoothapis.h>

#include "../Common/Assert.h"

#pragma comment(lib, "Bthprops.lib")

#define A2ARG(s) \
	s.Address.rgBytes[5], s.Address.rgBytes[4], s.Address.rgBytes[3],\
	s.Address.rgBytes[2], s.Address.rgBytes[1], s.Address.rgBytes[0]
#define B2ARG(s, x) s.f##x ? L#x : L"NOT " L#x
#define T2ARG(s, x) s.x.wYear, s.x.wMonth, s.x.wDay, s.x.wHour, s.x.wMinute, s.x.wSecond

HRESULT EnumBluetoothDevices()
{
	BLUETOOTH_DEVICE_SEARCH_PARAMS params = {sizeof(params), TRUE, TRUE, TRUE, TRUE, TRUE, 1};
	BLUETOOTH_DEVICE_INFO info = {sizeof(info)};

	auto hFind = BluetoothFindFirstDevice(&params, &info);
	WIN32_ASSERT(hFind);
	_putws(L"Address           Last seen           Last used           Name");
	do {
		wprintf_s(
			L"%02x:%02x:%02x:%02x:%02x:%02x %04d/%02d/%02d %02d:%02d:%02d %04d/%02d/%02d %02d:%02d:%02d %-20s %s, %s, %s\n",
			A2ARG(info), T2ARG(info, stLastSeen), T2ARG(info, stLastUsed), info.szName,
			B2ARG(info, Remembered), B2ARG(info, Authenticated), B2ARG(info, Connected));
	} while(BluetoothFindNextDevice(hFind, &info));
	auto hr = WIN32_EXPECT(GetLastError() == ERROR_NO_MORE_ITEMS);
	WIN32_ASSERT(BluetoothFindDeviceClose(hFind));

	return hr;
}
