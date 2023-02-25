#include <Windows.h>
#include <sal.h>
#include <bthsdpdef.h>
#include <bluetoothapis.h>

#include <string>
#include <sstream>

#include "../Common/Assert.h"

#pragma comment(lib, "Bthprops.lib")

static std::wstring ClassOfDevice(ULONG value);

// Struct defines Major and Minor Device Class.
struct MajorMinorDeviceClass {
	LPCWSTR major;			// Major Device Class.
	size_t minorSize;		// Array size of Minor Device Classes.
	const LPCWSTR* pMinor;	// Minor Device Classes array.

	// Function that returns Minor Device Class corresponding to ULONGLONG value.
	using MinorDeviceClassFunc = std::wstring(*)(const MajorMinorDeviceClass&, ULONGLONG);
	MinorDeviceClassFunc minorDeviceClassFunc;
};
static std::wstring DefaultMinorDeviceCalssFunc(const MajorMinorDeviceClass&, ULONGLONG);

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
			L"%02x:%02x:%02x:%02x:%02x:%02x %04d/%02d/%02d %02d:%02d:%02d %04d/%02d/%02d %02d:%02d:%02d %-20s %s, %s, %s\n"
			L"  Class of Device = %s\n",
			A2ARG(info), T2ARG(info, stLastSeen), T2ARG(info, stLastUsed), info.szName,
			B2ARG(info, Remembered), B2ARG(info, Authenticated), B2ARG(info, Connected),
			ClassOfDevice(info.ulClassofDevice).c_str());
	} while(BluetoothFindNextDevice(hFind, &info));
	auto hr = WIN32_EXPECT(GetLastError() == ERROR_NO_MORE_ITEMS);
	WIN32_ASSERT(BluetoothFindDeviceClose(hFind));

	return hr;
}

static const LPCWSTR MajorServiceClasses[] = {
	/*Bit*/
	/* 13*/ L"Limited Discoverable Mode",
	/* 14*/ L"LE audio",
	/* 15*/ L"Reserved for future use",
	/* 16*/ L"Positioning(Location identification)",
	/* 17*/ L"Networking",
	/* 18*/ L"Rnedering",
	/* 19*/ L"Capturing",
	/* 20*/ L"Object Transfer",
	/* 21*/ L"Audio",
	/* 22*/ L"Telephony",
	/* 23*/ L"Information"
};

static const LPCWSTR PhoneMinorDeviceClasses[] = {
	L"Uncategorized, code for device not assigned",
	L"Cellular",
	L"Cordless",
	L"Smatphone",
	L"Wired modem or void gateway",
	L"Common ISDN access",
};

static const LPCWSTR AudioVideoMinorDeviceClasses[] = {
	L"Uncategorized, code not assigned",
	L"Wearable Headset Device",
	L"Hands-free Device",
	L"(Reserved)",
	L"Microphone",
	L"Loudspeaker",
	L"Headphones",
	L"Portable Audio",
	L"Car Audio",
	L"Set-top box",
	L"HiFi Audio Device",
	L"VCR",
	L"Video Camera",
	L"Camcorder",
	L"Video Monitor",
	L"Video Display and Loudspeaker",
	L"Video Conferencing",
	L"(Reserved)",
	L"Gaming/Toy",
};

static const MajorMinorDeviceClass MajorDeviceClasses[] = {
	{L"Miscellaneous"},
	{L"Computer"},
	{L"Phone", ARRAYSIZE(PhoneMinorDeviceClasses), PhoneMinorDeviceClasses},
	{L"LAN/Network Access point"},
	{L"Audio/Video", ARRAYSIZE(AudioVideoMinorDeviceClasses), AudioVideoMinorDeviceClasses},
	{L"Peripheral"},
	{L"Imaging"},
	{L"Wearable"},
	{L"Toy"},
	{L"Health"},
	{L"Uncategorized: device code not specified"},
};

// Returns string representing MajorServiceClass, MajorDeviceClass and MinorDeviceClass for Class of Device value.
// See https://btprodspecificationrefs.blob.core.windows.net/assigned-numbers/Assigned%20Number%20Types/Assigned%20Numbers.pdf
std::wstring ClassOfDevice(ULONG value)
{
	std::wostringstream stream;
	stream << L"0x" << std::hex << value << L" : ";

	value >>= 2;
	auto minorDeviceClass = value & 0b111111;			// Bit 2 - 7(6 bits)
	value >>= 6;
	auto majorDeviceClass = value & 0b11111;			// Bit 8 - 12(5 bits)
	value >>= 5;
	auto majorServiceClass = value & 0b11111111111;		// Bit 13 - 23(11 bits)

	auto var = majorServiceClass;
	auto separator = L"";
	for(auto str : MajorServiceClasses) {
		if(var & 1) {
			stream << separator << str;
			separator = L",";
		}
		var >>= 1;
		if(var == 0) { break; /* No more bit is set.*/ }
	}

	stream << L"(0x" << std::hex << majorServiceClass << L") : ";
	auto index = (majorDeviceClass < ARRAYSIZE(MajorDeviceClasses)) ? majorDeviceClass : (ARRAYSIZE(MajorDeviceClasses) - 1);
	auto& deviceClass = MajorDeviceClasses[index];
	stream << deviceClass.major << L"(0x" << std::hex << majorDeviceClass << L") : ";
	auto func = deviceClass.minorDeviceClassFunc;
	if(!func) { func = DefaultMinorDeviceCalssFunc; }
	stream << func(deviceClass, minorDeviceClass);

	return stream.str();
}

// Returns Minor Device Class corresponding to the value.
// The value is used as index of array that consits of Minor Devie Class strings.
std::wstring DefaultMinorDeviceCalssFunc(const MajorMinorDeviceClass& classes, ULONGLONG value)
{
	std::wostringstream stream;
	if(classes.minorSize && classes.pMinor) {
		if(value < classes.minorSize) {
			stream << classes.pMinor[value];
		} else {
			stream << L"Out of range";
		}
	}

	stream << L"(0x" << std::hex << value << L")";
	return stream.str();
}
