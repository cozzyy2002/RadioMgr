#include "BluetoothCommon.h"

#include "../Common/SafeHandle.h"
#include "../Common/Assert.h"

static void HandleDeleteFunc(HANDLE h) { WIN32_EXPECT(CloseHandle(h)); }

HRESULT EnumBluetoothRadios()
{
	BLUETOOTH_FIND_RADIO_PARAMS params{sizeof(params)};
	HANDLE hRadio;
	auto hFind = BluetoothFindFirstRadio(&params, &hRadio);
	WIN32_ASSERT(hFind);
	do {
		SafeHandle<HANDLE, HandleDeleteFunc> _h(hRadio);

		BLUETOOTH_RADIO_INFO info{sizeof(info)};
		HR_ASSERT_OK(HRESULT_FROM_WIN32(BluetoothGetRadioInfo(hRadio, &info)));
		BTH_MFG_3COM;
		wprintf_s(
			L"%02x:%02x:%02x:%02x:%02x:%02x %s Manufacturer 0x%04x, Subversion 0x%04x\n"
			L"  Class of Device = %s\n",
			A2ARG(info, address), info.szName, info.manufacturer, info.lmpSubversion,
			ClassOfDevice(info.ulClassofDevice).c_str());
	} while(BluetoothFindNextRadio(hFind, &hRadio));
	auto hr = WIN32_EXPECT(GetLastError() == ERROR_NO_MORE_ITEMS);
	WIN32_ASSERT(BluetoothFindRadioClose(hFind));

	return hr;
}
