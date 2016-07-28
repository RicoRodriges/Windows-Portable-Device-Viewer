#include "WPDevice.h"

WPDEnumerator::WPDEnumerator() {
	deviceManager = nullptr;
	DeviceCount = 0;

	try {
		hr = CoCreateInstance(CLSID_PortableDeviceManager,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&deviceManager));
		if (FAILED(hr)) throw 1;

		hr = deviceManager->GetDevices(nullptr, &DeviceCount);
		if (FAILED(hr)) throw 2;
	} catch (int a) {
		if (a > 1)
			deviceManager->Release();
		DeviceCount = 0;
		deviceManager = nullptr;
		setError(a, hr);
		return;
	}

	setError(0);
}

WPDEnumerator::~WPDEnumerator() {
	if (deviceManager)
		deviceManager->Release();
}

int WPDEnumerator::getError(HRESULT *r) {
	if(r)
		*r=hr;
	return error;
}

void WPDEnumerator::setError(int err, HRESULT r) { error = err; hr = r; }

int WPDEnumerator::getDeviceCount() { return DeviceCount; }

bool WPDEnumerator::refreshDeviceList() {
	if (!deviceManager) { setError(1); return false; }

	HRESULT hr;
	try {
		hr = deviceManager->RefreshDeviceList();
		if (FAILED(hr)) throw 2;

		hr = deviceManager->GetDevices(nullptr, &DeviceCount);
		if (FAILED(hr)) throw 3;
	} catch (int a) {
		deviceManager->Release();
		deviceManager = nullptr;
		DeviceCount = 0;
		setError(a, hr);
		return false;
	}

	setError(0);
	return true;
}

void WPDEnumerator::clearDevicePaths(wchar_t **paths) {
	for (DWORD index = 0; index < DeviceCount; index++)
		CoTaskMemFree(paths[index]);
	delete [] paths;
}

bool WPDEnumerator::getDevicePath(int deviceIndex, std::wstring *path) {
	if (!deviceManager) { setError(1); return false; }
	if (deviceIndex >= DeviceCount || DeviceCount < 1) { setError(2); return false; }
	if (!path) { setError(3); return false; }

	wchar_t **pnpDeviceIDs = new wchar_t*[DeviceCount];
	if (!pnpDeviceIDs) { setError(4); return false; }

	DWORD retrievedDeviceIDCount = DeviceCount;
	HRESULT hr = deviceManager->GetDevices(pnpDeviceIDs, &retrievedDeviceIDCount);
	if (FAILED(hr)) { delete[] pnpDeviceIDs; setError(5, hr); return false; }

	if (retrievedDeviceIDCount <= deviceIndex) { clearDevicePaths(pnpDeviceIDs); setError(6); return false; }

	*path = pnpDeviceIDs[deviceIndex];
	clearDevicePaths(pnpDeviceIDs);
	setError(0);
	return true;
}

bool WPDEnumerator::getDeviceStringValue(const std::wstring &path, std::wstring *str, int param) {
	if (!deviceManager) { setError(1); return false; }
	if (!str) { setError(2); return false; }

	DWORD length = 0;
	HRESULT hr;
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
	if (FAILED(hr)) { setError(3, hr); return false; }

	if (length == 0) {
		*str = L"";
		setError(0);
		return true;
	}

	wchar_t *temp = new wchar_t[length + 1];
	if (!temp) { setError(4); return false; }

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
	if (FAILED(hr)) { delete[] temp; setError(5, hr); return false; }

	*str = temp;
	delete[] temp;
	setError(0);
	return true;
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





WPDevice::WPDevice(const std::wstring &path) {
	device = nullptr;
	content = nullptr;
	properties = nullptr;
	resources = nullptr;
	keys = nullptr;

	HRESULT hr;
	IPortableDeviceValues *clientInformation = nullptr;
	try {
		hr = CoCreateInstance(CLSID_PortableDeviceFTM,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&device));
		if (FAILED(hr)) throw 1;

		hr = CoCreateInstance(CLSID_PortableDeviceValues,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&clientInformation));
		if (FAILED(hr)) throw 2;

		if (FAILED(hr = clientInformation->SetStringValue(WPD_CLIENT_NAME, L"WPD Viewer")) ||
			FAILED(hr = clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1)) ||
			FAILED(hr = clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 0)) ||
			FAILED(hr = clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 1)) ||
			FAILED(hr = clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ)) ||
			FAILED(hr = clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SHARE_MODE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)) ||
			FAILED(hr = clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION)))
			throw 3;

		hr = device->Open(path.c_str(), clientInformation);
		if (FAILED(hr)) throw 4;
		clientInformation->Release();

		hr = device->Content(&content);
		if (FAILED(hr))	throw 5;

	} catch (int a) {
		if (a >= 5)
			device->Close();
		if (a == 3 || a == 4)
			clientInformation->Release();
		if (a >= 1)
			device->Release();
		device = nullptr;
		content = nullptr;
		setError(a, hr);
		return;
	}
	setError(0);
	
}

WPDevice::~WPDevice() {
	if (keys)
		keys->Release();
	if (resources)
		resources->Release();
	if (properties)
		properties->Release();
	if (content)
		content->Release();
	if (device) {
		device->Close();
		device->Release();
	}
}

void WPDevice::setError(int err, HRESULT r) {error = err; hr = r; }

int WPDevice::getError(HRESULT *r) {
	if(r)
		*r=hr;
	return error;
}

HRESULT WPDevice::createProperties() {
	if (!properties) {
		HRESULT hr = content->Properties(&properties);
		if (FAILED(hr))
			properties = nullptr;
		return hr;
	}
	return 0;
}

HRESULT WPDevice::createResources() {
	if (!resources) {
		HRESULT hr = content->Transfer(&resources);
		if (FAILED(hr))
			resources = nullptr;
		return hr;
	}
	return 0;
}

HRESULT WPDevice::createKeyCollection() {
	if (!keys) {
		HRESULT hr;
		hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&keys));
		if (FAILED(hr))
			keys = nullptr;
		return hr;
	}
	keys->Clear();
	return 0;
}

bool WPDevice::getFileEnumerator(const std::wstring *path, WPDFileEnumerator *enumerator) {
	if (!device) { setError(1); return false; }
	if (!enumerator) { setError(2); return false; }

	HRESULT hr;
	IEnumPortableDeviceObjectIDs *enumObjectIDs;

	if (!path) {
		hr = content->EnumObjects(0,
			WPD_DEVICE_OBJECT_ID,
			nullptr,
			&enumObjectIDs);
	} else {
		hr = content->EnumObjects(0,
			path->c_str(),
			nullptr,
			&enumObjectIDs);
	}
	if (FAILED(hr)) { setError(3, hr); return false; }

	enumerator->setEnumerator(enumObjectIDs);
	setError(0);
	return true;
}

bool WPDevice::getFileDateParam(const std::wstring &path, SYSTEMTIME *date, const PROPERTYKEY &key) {
	if (!device) { setError(1); return false; }
	if (!date) { setError(2); return false; }

	HRESULT hr;
	hr = createProperties();
	if (FAILED(hr)) { setError(3, hr); return false; }

	hr = createKeyCollection();
	if (FAILED(hr)) { setError(4, hr); return false; }

	hr = keys->Add(key);
	if (FAILED(hr)) { setError(5, hr); return false; }

	IPortableDeviceValues *objectProperties;
	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) { setError(6, hr); return false; }

	PROPVARIANT temp;
	hr = objectProperties->GetValue(key, &temp);
	if (FAILED(hr)) {
		objectProperties->Release();
		setError(7, hr); return false;
	}
	objectProperties->Release();

	if (!VariantTimeToSystemTime(temp.date, date)) {
		PropVariantClear(&temp);
		setError(8, GetLastError());
		return false;
	}

	PropVariantClear(&temp);
	setError(0);
	return true;
}

bool WPDevice::getFileInt8Param(const std::wstring &path, ULONGLONG *num, const PROPERTYKEY &key) {
	if (!device) { setError(1); return false; }
	if (!num) { setError(2); return false; }

	HRESULT hr;
	hr = createProperties();
	if (FAILED(hr)) { setError(3, hr); return false; }

	hr = createKeyCollection();
	if (FAILED(hr)) { setError(4, hr); return false; }

	hr = keys->Add(key);
	if (FAILED(hr)) { setError(5, hr); return false; }

	IPortableDeviceValues *objectProperties;
	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) { setError(6, hr); return false; }

	hr = objectProperties->GetUnsignedLargeIntegerValue(key, num);
	if (FAILED(hr)) {
		objectProperties->Release();
		setError(7, hr);
		return false;
	}

	objectProperties->Release();
	setError(0);
	return true;
}

bool WPDevice::getFileCharParam(const std::wstring &path, std::wstring *str, const PROPERTYKEY &key) {
	if (!device) { setError(1); return false; }
	if (!str) { setError(2); return false; }

	HRESULT hr;
	hr = createProperties();
	if (FAILED(hr)) { setError(3, hr); return false; }

	hr = createKeyCollection();
	if (FAILED(hr)) { setError(4, hr); return false; }

	hr = keys->Add(key);
	if (FAILED(hr)) { setError(5, hr); return false; }

	IPortableDeviceValues *objectProperties;
	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) { setError(6, hr); return false; }

	wchar_t *temp;
	hr = objectProperties->GetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, &temp);
	if (FAILED(hr)) {
		objectProperties->Release();
		setError(7, hr);
		return false;
	}

	objectProperties->Release();
	*str = temp;
	CoTaskMemFree(temp);
	setError(0);
	return true;
}

bool WPDevice::getFileGUIDParam(const std::wstring &path, GUID *guid, const PROPERTYKEY &key) {
	if (!device) { setError(1); return false; }
	if (!guid) { setError(2); return false; }

	HRESULT hr;
	hr = createProperties();
	if (FAILED(hr)) { setError(3, hr); return false; }

	hr = createKeyCollection();
	if (FAILED(hr)) { setError(4, hr); return false; }

	hr = keys->Add(key);
	if (FAILED(hr)) { setError(5, hr); return false; }

	IPortableDeviceValues *objectProperties;
	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) { setError(6, hr); return false; }

	hr = objectProperties->GetGuidValue(key, guid);
	if (FAILED(hr)) {
		objectProperties->Release();
		setError(7, hr); return false;
	}
	objectProperties->Release();
	setError(0);
	return true;
}

bool WPDevice::getSpaceFree(const std::wstring &path, unsigned long long *size) {
	return getFileInt8Param(path, size, WPD_STORAGE_FREE_SPACE_IN_BYTES);
}

bool WPDevice::getSpace(const std::wstring &path, unsigned long long *size) {
	return getFileInt8Param(path, size, WPD_STORAGE_CAPACITY);
}

bool WPDevice::getFileSystem(const std::wstring &path, std::wstring *str) {
	return getFileCharParam(path, str, WPD_STORAGE_FILE_SYSTEM_TYPE);
}

bool WPDevice::getFileSize(const std::wstring &path, unsigned long long *size) {
	return getFileInt8Param(path, size, WPD_OBJECT_SIZE);
}

bool WPDevice::getFileName(const std::wstring &path, std::wstring *str) {
	bool result;
	result = getFileCharParam(path, str, WPD_OBJECT_ORIGINAL_FILE_NAME);
	if (!result)
		result = getFileCharParam(path, str, WPD_OBJECT_NAME);
	return result;
}

bool WPDevice::getFileDateModified(const std::wstring &path, SYSTEMTIME *dateModif) {
	return getFileDateParam(path, dateModif, WPD_OBJECT_DATE_MODIFIED);
}

bool WPDevice::getFileDateCreated(const std::wstring &path, SYSTEMTIME *dateCreate) {
	return getFileDateParam(path, dateCreate, WPD_OBJECT_DATE_CREATED);
}

bool WPDevice::getObjectType(const std::wstring &path, GUID *guid) {
	return getFileGUIDParam(path, guid, WPD_OBJECT_CONTENT_TYPE);
}

bool WPDevice::getFunctionalObjectCategoty(const std::wstring &path, GUID *guid) {
	return getFileGUIDParam(path, guid, WPD_FUNCTIONAL_OBJECT_CATEGORY);
}

bool WPDevice::copyFile(const std::wstring &path, const std::wstring &newFile, const FILETIME *dateCreate, const FILETIME *dateModif) {
	if (!device) { setError(1); return false; }

	HRESULT hr;
	IStream *objectDataStream = 0;
	HANDLE finalFile = 0;
	try {
		hr = createResources();
		if (FAILED(hr)) throw 2;

		DWORD optimalTransferSizeBytes;
		hr = resources->GetStream(path.c_str(), WPD_RESOURCE_DEFAULT, STGM_READ, &optimalTransferSizeBytes, &objectDataStream);
		if (FAILED(hr)) throw 3;

		finalFile = CreateFileW(newFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (finalFile == INVALID_HANDLE_VALUE) {
			hr = GetLastError();
			throw 4;
		}

		hr = StreamCopy(finalFile, objectDataStream, optimalTransferSizeBytes);
		if (FAILED(hr)) throw 5;

		objectDataStream->Release();

		if (dateCreate || dateModif) {
			if (!SetFileTime(finalFile, dateCreate, dateModif, dateModif)) {
				hr = GetLastError();
				throw 6;
			}
		}
		CloseHandle(finalFile);
	} catch (int a) {
		if (a >= 5)
			CloseHandle(finalFile);
		if (a == 4 || a == 5)
			objectDataStream->Release();
		setError(a, hr);
		return false;
	}

	setError(0);
	return true;
}

HRESULT WPDevice::StreamCopy(HANDLE destStream, IStream *sourceStream, DWORD transferSizeBytes) {
	char* objectData = new char[transferSizeBytes];
	if (!objectData)
		return -1;

	DWORD bytesRead;
	DWORD bytesWritten;

	HRESULT hr;
	do
	{
		hr = sourceStream->Read(objectData, transferSizeBytes, &bytesRead);
		if (FAILED(hr)) {
			hr = 0;
			goto exit;
		}

		if (!WriteFile(destStream, objectData, bytesRead, &bytesWritten, nullptr)) {
			hr = -2;
			goto exit;
		}

	} while (SUCCEEDED(hr) && (bytesRead > 0));

exit:
	delete[] objectData;
	return hr;
}




WPDFileEnumerator::WPDFileEnumerator() {
	enumObjectIDs = nullptr;
	setError(0);
}

WPDFileEnumerator::~WPDFileEnumerator() {
	if(enumObjectIDs)
		enumObjectIDs->Release();
}

void WPDFileEnumerator::setError(int err, HRESULT r) {error = err; hr = r; }

void WPDFileEnumerator::setEnumerator(IEnumPortableDeviceObjectIDs *enumerator) {
	if (enumObjectIDs)
		enumObjectIDs->Release();
	enumObjectIDs = enumerator;
	setError(0);
}

int WPDFileEnumerator::getError(HRESULT *r) {
	if(r)
		*r=hr;
	return error;
}

int WPDFileEnumerator::getNextFile(std::wstring *paths, int pathsNum) {
	if (!enumObjectIDs) { setError(1); return 0; }
	if (!paths || pathsNum < 1) { setError(2); return 0; }

	wchar_t **temp = new wchar_t*[pathsNum];
	if (!temp) { setError(3); return 0; }

	ULONG result = 0;
	HRESULT hr;
	hr = enumObjectIDs->Next(pathsNum, temp, &result);
	if (FAILED(hr)) { delete[] temp; setError(4, hr); return 0; }

	for (ULONG index = 0; index < result; index++) {
		paths[index] = temp[index];
		CoTaskMemFree(temp[index]);
	}
	delete[] temp;
	setError(0);
	return result;
}

bool WPDFileEnumerator::Skip(int num) {
	if (!enumObjectIDs) { setError(1); return false; }
	if (num < 1) { setError(2); return false; }
	
	HRESULT hr;
	hr = enumObjectIDs->Skip(num);
	if (FAILED(hr)) { setError(3, hr); return false; }

	setError(0);
	return true;
}

bool WPDFileEnumerator::Reset() {
	if (!enumObjectIDs) { setError(1); return false; }
	
	HRESULT hr;
	hr = enumObjectIDs->Reset();
	if (FAILED(hr)) { setError(2, hr); return false; }

	setError(0);
	return true;
}