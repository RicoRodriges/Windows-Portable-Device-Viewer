#include "WPDevice.h"

WPDevice::WPDevice(const std::wstring &path) {
	device = nullptr;
	content = nullptr;
	properties = nullptr;
	resources = nullptr;
	keys = nullptr;

	IPortableDeviceValues *clientInformation = nullptr;
	hr = CoCreateInstance(CLSID_PortableDeviceFTM,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&device));
	if (hr != S_OK) throw WPDException(hr, "An instance of PortableDevice class wasn't created");

	hr = CoCreateInstance(CLSID_PortableDeviceValues,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&clientInformation));
	if (hr != S_OK) throw WPDException(hr, "An instance of PortableDeviceValues class wasn't created");

	clientInformation->SetStringValue(WPD_CLIENT_NAME, L"WPD Viewer");
	clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1);
	clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 0);
	clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 1);
	clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
	clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SHARE_MODE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
	clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);

	hr = device->Open(path.c_str(), clientInformation);
	if (hr != S_OK) throw WPDException(hr, "Couldn't open a connection between the application and the device");
	clientInformation->Release();
	clientInformation = nullptr;

	hr = device->Content(&content);
	if (FAILED(hr))	throw WPDException(hr, "Couldn't get a content");
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

HRESULT WPDevice::getError() { return hr; }

void WPDevice::createProperties() {
	if (!properties) {
		hr = content->Properties(&properties);
		if (FAILED(hr)) throw WPDException(hr, "Couldn't get a properties");
	}
}

void WPDevice::createResources() {
	if (!resources) {
		hr = content->Transfer(&resources);
		if (FAILED(hr)) throw WPDException(hr, "Couldn't get a resources");
	}
}

void WPDevice::createKeyCollection() {
	if (!keys) {
		hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&keys));
		if (FAILED(hr)) throw WPDException(hr, "An instance of PortableDeviceKeyCollection class wasn't created");
	}
	keys->Clear();
}

void WPDevice::getFileEnumerator(const std::wstring &path, WPDFileEnumerator &enumerator) {
	IEnumPortableDeviceObjectIDs *enumObjectIDs = nullptr;

	hr = content->EnumObjects(0,
		(path == L"") ? WPD_DEVICE_OBJECT_ID : path.c_str(),
		nullptr,
		&enumObjectIDs);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't get a enumerator");

	enumerator.setEnumerator(enumObjectIDs);
}

bool WPDevice::getFileDateParam(const std::wstring &path, SYSTEMTIME *date, const PROPERTYKEY &key) {
	bool result = false;
	IPortableDeviceValues *objectProperties = nullptr;
	PROPVARIANT temp;

	if (!date) throw WPDException(0, "Invalid arguments");

	createProperties();
	createKeyCollection();
	hr = keys->Add(key);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't add a property key to the collection");

	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) goto error;

	hr = objectProperties->GetValue(key, &temp);
	if (FAILED(hr)) goto error;

	if (!VariantTimeToSystemTime(temp.date, date)) {
		hr = GetLastError();
		goto error;
	}

	result = true;
error:
	if (objectProperties)
		objectProperties->Release();
	PropVariantClear(&temp);
	return result;
}

bool WPDevice::getFileInt8Param(const std::wstring &path, ULONGLONG *num, const PROPERTYKEY &key) {
	bool result = false;
	IPortableDeviceValues *objectProperties = nullptr;

	if (!num) throw WPDException(0, "Invalid arguments");

	createProperties();
	createKeyCollection();
	hr = keys->Add(key);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't add a property key to the collection");

	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) goto error;

	hr = objectProperties->GetUnsignedLargeIntegerValue(key, num);
	if (FAILED(hr)) goto error;

	result = true;
error:
	if (objectProperties)
		objectProperties->Release();
	return result;
}

bool WPDevice::getFileCharParam(const std::wstring &path, std::wstring *str, const PROPERTYKEY &key) {
	bool result = false;
	IPortableDeviceValues *objectProperties = nullptr;
	wchar_t *temp = nullptr;
	if (!str) throw WPDException(0, "Invalid arguments");

	createProperties();
	createKeyCollection();
	hr = keys->Add(key);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't add a property key to the collection");

	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) goto error;

	hr = objectProperties->GetStringValue(key, &temp);
	if (FAILED(hr)) goto error;

	*str = temp;
	result = true;
error:
	if (objectProperties)
		objectProperties->Release();
	if (temp)
		CoTaskMemFree(temp);
	return result;
}

bool WPDevice::getFileGUIDParam(const std::wstring &path, GUID *guid, const PROPERTYKEY &key) {
	bool result = false;
	IPortableDeviceValues *objectProperties = nullptr;
	if (!guid) throw WPDException(0, "Invalid arguments");

	createProperties();
	createKeyCollection();
	hr = keys->Add(key);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't add a property key to the collection");

	hr = properties->GetValues(path.c_str(), keys, &objectProperties);
	if (FAILED(hr)) goto error;

	hr = objectProperties->GetGuidValue(key, guid);
	if (FAILED(hr)) goto error;

	result = true;
error:
	if (objectProperties)
		objectProperties->Release();
	return result;
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
	IStream *objectDataStream = nullptr;
	HANDLE finalFile = INVALID_HANDLE_VALUE;

	createResources();

	DWORD optimalTransferSizeBytes;
	hr = resources->GetStream(path.c_str(), WPD_RESOURCE_DEFAULT, STGM_READ, &optimalTransferSizeBytes, &objectDataStream);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't get a stream");

	finalFile = CreateFileW(newFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (finalFile == INVALID_HANDLE_VALUE) {
		hr = GetLastError();
		throw WPDException(hr, "Couldn't create a file");
	}

	StreamCopy(finalFile, objectDataStream, optimalTransferSizeBytes);

	if (dateCreate || dateModif)
		SetFileTime(finalFile, dateCreate, dateModif, dateModif);

	if (objectDataStream)
		objectDataStream->Release();
	if (finalFile != INVALID_HANDLE_VALUE)
		CloseHandle(finalFile);
	return true;
}

void WPDevice::StreamCopy(HANDLE destStream, IStream *sourceStream, DWORD transferSizeBytes) {
	char* objectData = new char[transferSizeBytes];

	DWORD bytesRead;
	DWORD bytesWritten;

	HRESULT hr;
	while (1) {
		hr = sourceStream->Read(objectData, transferSizeBytes, &bytesRead);
		if (FAILED(hr)) throw WPDException(hr, "Couldn't read a stream");
		if (bytesRead == 0) break;

		if (!WriteFile(destStream, objectData, bytesRead, &bytesWritten, nullptr)) {
			hr = GetLastError();
			throw WPDException(hr, "Couldn't write to file");
		}
	}

	delete[] objectData;
}




WPDFileEnumerator::WPDFileEnumerator() : enumObjectIDs(nullptr) {}

WPDFileEnumerator::~WPDFileEnumerator() {
	if (enumObjectIDs)
		enumObjectIDs->Release();
}

void WPDFileEnumerator::setEnumerator(IEnumPortableDeviceObjectIDs *enumerator) {
	if (enumObjectIDs)
		enumObjectIDs->Release();
	enumObjectIDs = enumerator;
}

HRESULT WPDFileEnumerator::getError() { return hr; }

int WPDFileEnumerator::getNextFile(std::wstring *paths, int pathsNum) {
	if (!enumObjectIDs) throw WPDException(0, "Enumerator is null");
	if (!paths || pathsNum < 1) throw WPDException(0, "Invalid arguments");

	wchar_t **temp = new wchar_t*[pathsNum];

	ULONG result = 0;
	hr = enumObjectIDs->Next(pathsNum, temp, &result);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't get the next object");

	for (ULONG index = 0; index < result; index++) {
		paths[index] = temp[index];
		CoTaskMemFree(temp[index]);
	}
	delete[] temp;
	return result;
}

bool WPDFileEnumerator::Skip(int num) {
	if (!enumObjectIDs) throw WPDException(0, "Enumerator is null");
	if (num < 1) throw WPDException(0, "Invalid arguments");

	hr = enumObjectIDs->Skip(num);
	if (FAILED(hr)) throw WPDException(hr, "Couldn't skip a specified number of objects");

	if (hr == S_OK)
		return true;
	else
		return false;
}

void WPDFileEnumerator::Reset() {
	if (!enumObjectIDs) throw WPDException(0, "Enumerator is null");

	hr = enumObjectIDs->Reset();
	if (FAILED(hr)) throw WPDException(hr, "Couldn't reset the enumeration sequence to the beginning");
}