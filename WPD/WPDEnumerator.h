#pragma once
#include <PortableDeviceApi.h>
//#include <PortableDevice.h>
#include <string>
#include "WPDException.h"
#pragma comment(lib, "PortableDeviceGuids.lib")

class WPDEnumerator {
	HRESULT hr;
	IPortableDeviceManager *deviceManager;
	DWORD DeviceCount;
	bool getDeviceStringValue(const std::wstring &path, std::wstring *str, int param);
	void clearDevicePaths(wchar_t **path, DWORD size);
public:
	WPDEnumerator();
	~WPDEnumerator();
	void refreshDeviceList();
	HRESULT getError();
	DWORD getDeviceCount();
	bool getDevicePath(int deviceIndex, std::wstring *path);
	bool getFriendlyName(const std::wstring &path, std::wstring *str);
	bool getManufacturer(const std::wstring &path, std::wstring *str);
	bool getDescription(const std::wstring &path, std::wstring *str);
};