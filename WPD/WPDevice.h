#include <PortableDeviceApi.h>
#include <PortableDevice.h>
#include <string>
#pragma comment(lib, "PortableDeviceGuids.lib")

class WPDEnumerator {
	int error; HRESULT hr;
	IPortableDeviceManager *deviceManager;
	DWORD DeviceCount;
	bool getDeviceStringValue(const std::wstring &path, std::wstring *str, int param);
	void setError(int err, HRESULT r = 0);
	void clearDevicePaths(wchar_t **path);
public:
	WPDEnumerator();
	~WPDEnumerator();
	bool refreshDeviceList();
	int getError(HRESULT *r = nullptr);
	int getDeviceCount();
	bool getDevicePath(int deviceIndex, std::wstring *path);
	bool getFriendlyName(const std::wstring &path, std::wstring *str);
	bool getManufacturer(const std::wstring &path, std::wstring *str);
	bool getDescription(const std::wstring &path, std::wstring *str);
	
};

class WPDFileEnumerator {
	friend class WPDevice;
	int error; HRESULT hr;
	IEnumPortableDeviceObjectIDs *enumObjectIDs;
	void setError(int err, HRESULT r = 0);
	void setEnumerator(IEnumPortableDeviceObjectIDs *enumerator);
public:
	WPDFileEnumerator();
	~WPDFileEnumerator();
	// 'paths' - array with 'pathsNum' items
	int getNextFile(std::wstring *paths, int pathsNum);
	bool Skip(int num);
	bool Reset();
	int getError(HRESULT *r = NULL);
};

class WPDevice {
	int error; HRESULT hr;
	IPortableDevice *device;
	IPortableDeviceContent *content;
	IPortableDeviceProperties *properties;
	IPortableDeviceResources *resources;
	IPortableDeviceKeyCollection *keys;
	void setError(int err, HRESULT r = 0);
	HRESULT StreamCopy(HANDLE destStream, IStream *sourceStream, DWORD transferSizeBytes);
	bool getFileDateParam(const std::wstring &path, SYSTEMTIME *date, const PROPERTYKEY &key);
	bool getFileInt8Param(const std::wstring &path, ULONGLONG *num, const PROPERTYKEY &key);
	bool getFileCharParam(const std::wstring &path, std::wstring *str, const PROPERTYKEY &key);
	bool getFileGUIDParam(const std::wstring &path, GUID *guid, const PROPERTYKEY &key);
	HRESULT createProperties();
	HRESULT createResources();
	HRESULT createKeyCollection();
public:
	WPDevice(const std::wstring &path);
	~WPDevice();
	int getError(HRESULT *r = NULL);
	bool getFileEnumerator(const std::wstring *path, WPDFileEnumerator *enumerator);

	// Return WPD_CONTENT_TYPE_*** GUID
	// See more https://msdn.microsoft.com/en-us/library/windows/hardware/ff597558(v=vs.85).aspx
	bool getObjectType(const std::wstring &path, GUID *guid);

	// If object type is WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT then return WPD_FUNCTIONAL_CATEGORY_*** GUID
	// See more https://msdn.microsoft.com/en-us/library/windows/desktop/dd389028(v=vs.85).aspx
	bool getFunctionalObjectCategoty(const std::wstring &path, GUID *guid);

	bool getFileName(const std::wstring &path, std::wstring *str);

	/* Operation with functional objects */
	bool getSpaceFree(const std::wstring &path, unsigned long long *size);
	bool getSpace(const std::wstring &path, unsigned long long *size);
	bool getFileSystem(const std::wstring &path, std::wstring *str);

	/* Operation with files */
	bool getFileSize(const std::wstring &path, unsigned long long *size);
	bool getFileDateModified(const std::wstring &path, SYSTEMTIME *dateModif);
	bool getFileDateCreated(const std::wstring &path, SYSTEMTIME *dateCreate);
	bool copyFile(const std::wstring &path, const std::wstring &newFile, const FILETIME *dateCreate = nullptr, const FILETIME *dateModif = nullptr);
};