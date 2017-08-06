#include "WPDEnumerator.h"

WPDEnumerator::WPDEnumerator() {
	deviceManager = nullptr;
	DeviceCount = 0;

	hr = CoCreateInstance(CLSID_PortableDeviceManager,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&deviceManager));
	if (hr != S_OK) throw WPDException(hr, "An instance of PortableDeviceManager class wasn't created");

	hr = deviceManager->GetDevices(nullptr, &DeviceCount);
}

WPDEnumerator::~WPDEnumerator() {
	if (deviceManager)
		deviceManager->Release();
}

HRESULT WPDEnumerator::getError() { return hr; }

DWORD WPDEnumerator::getDeviceCount() { return DeviceCount; }

void WPDEnumerator::refreshDeviceList() {
	deviceManager->RefreshDeviceList();
	hr = deviceManager->GetDevices(nullptr, &DeviceCount);
}

void WPDEnumerator::clearDevicePaths(wchar_t **paths, DWORD size) {
	for (DWORD index = 0; index < size; index++)
		CoTaskMemFree(paths[index]);
}

bool WPDEnumerator::getDevicePath(int deviceIndex, std::wstring *path) {
	bool result = false;
	wchar_t **pnpDeviceIDs = nullptr;
	DWORD retrievedDeviceIDCount = 0;

	if (!path || deviceIndex < 0) throw WPDException(0, "Invalid arguments");
	if (DeviceCount < 1) return false;

	pnpDeviceIDs = new wchar_t*[DeviceCount];
	retrievedDeviceIDCount = DeviceCount;
	hr = deviceManager->GetDevices(pnpDeviceIDs, &retrievedDeviceIDCount);
	if (FAILED(hr) || retrievedDeviceIDCount <= (DWORD)deviceIndex) goto error;

	*path = pnpDeviceIDs[deviceIndex];
	result = true;
error:
	if (pnpDeviceIDs) {
		if (retrievedDeviceIDCount > 0)
			clearDevicePaths(pnpDeviceIDs, retrievedDeviceIDCount);
		delete[] pnpDeviceIDs;
	}
	return result;
}

bool WPDEnumerator::getDeviceStringValue(const std::wstring &path, std::wstring *str, int param) {
	DWORD length = 0;
	wchar_t *temp = nullptr;
	bool result = false;

	if (!str) throw WPDException(0, "Invalid arguments");

	switch (param) {
	case 0:
		hr = deviceManager->GetDeviceFriendlyName(path.c_str(), nullptr, &length);
		break;
	case 1:
		hr = deviceManager->GetDeviceManufacturer(path.c_str(), nullptr, &length);
		break;
	default:
		hr = deviceManager->GetDeviceDescription(path.c_str(), nullptr, &length);
		break;
	}
	if (FAILED(hr)) return false;

	if (length == 0) {
		*str = L"";
		return true;
	}

	temp = new wchar_t[length];

	switch (param) {
	case 0:
		hr = deviceManager->GetDeviceFriendlyName(path.c_str(), temp, &length);
		break;
	case 1:
		hr = deviceManager->GetDeviceManufacturer(path.c_str(), temp, &length);
		break;
	default:
		hr = deviceManager->GetDeviceDescription(path.c_str(), temp, &length);
		break;
	}
	if (FAILED(hr)) goto error;

	*str = temp;
	result = true;
error:
	if (temp)
		delete[] temp;
	return result;
}

bool WPDEnumerator::getFriendlyName(const std::wstring &path, std::wstring *str) {
	return getDeviceStringValue(path, str, 0);
}

bool WPDEnumerator::getManufacturer(const std::wstring &path, std::wstring *str) {
	return getDeviceStringValue(path, str, 1);
}

bool WPDEnumerator::getDescription(const std::wstring &path, std::wstring *str) {
	return getDeviceStringValue(path, str, 2);
}