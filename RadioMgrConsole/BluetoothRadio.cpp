#include "BluetoothCommon.h"

#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

static HRESULT hr;
static void HandleDeleteFunc(HANDLE h) { 
	hr = WIN32_EXPECT(CloseHandle(h));
}

HRESULT EnumBluetoothRadios()
{
	BLUETOOTH_FIND_RADIO_PARAMS params{sizeof(params)};
	HANDLE _hRadio;
	auto hFind = BluetoothFindFirstRadio(&params, &_hRadio);
	WIN32_ASSERT(hFind);
	do {
		SafeHandle<HANDLE, HandleDeleteFunc> hRadio(_hRadio);

		BLUETOOTH_RADIO_INFO info{sizeof(info)};
		HR_ASSERT_OK(HRESULT_FROM_WIN32(BluetoothGetRadioInfo(hRadio, &info)));
		BTH_MFG_3COM;
		wprintf_s(L"%02x:%02x:%02x:%02x:%02x:%02x %s Manufacturer 0x%04x, Subversion 0x%04x\n",
			A2ARG(info, address), info.szName, info.manufacturer, info.lmpSubversion);
	} while(BluetoothFindNextRadio(hFind, &_hRadio));
	auto hr = WIN32_EXPECT(GetLastError() == ERROR_NO_MORE_ITEMS);
	WIN32_ASSERT(BluetoothFindRadioClose(hFind));

	return hr;
}
