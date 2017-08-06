#include <PortableDeviceApi.h>
#include <PortableDevice.h>
#include <string>
#include "WPDException.h"
#pragma comment(lib, "PortableDeviceGuids.lib")

class WPDFileEnumerator {
	friend class WPDevice;
	HRESULT hr;
	IEnumPortableDeviceObjectIDs *enumObjectIDs;
	void setEnumerator(IEnumPortableDeviceObjectIDs *enumerator);
public:
	WPDFileEnumerator();
	~WPDFileEnumerator();
	// 'paths' - array with 'pathsNum' items
	int getNextFile(std::wstring *paths, int pathsNum);
	bool Skip(int num);
	void Reset();
	HRESULT getError();
};

class WPDevice {
	HRESULT hr;
	IPortableDevice *device;
	IPortableDeviceContent *content;
	IPortableDeviceProperties *properties;
	IPortableDeviceResources *resources;
	IPortableDeviceKeyCollection *keys;
	static void StreamCopy(HANDLE destStream, IStream *sourceStream, DWORD transferSizeBytes);
	bool getFileDateParam(const std::wstring &path, SYSTEMTIME *date, const PROPERTYKEY &key);
	bool getFileInt8Param(const std::wstring &path, ULONGLONG *num, const PROPERTYKEY &key);
	bool getFileCharParam(const std::wstring &path, std::wstring *str, const PROPERTYKEY &key);
	bool getFileGUIDParam(const std::wstring &path, GUID *guid, const PROPERTYKEY &key);
	void createProperties();
	void createResources();
	void createKeyCollection();
public:
	WPDevice(const std::wstring &path);
	~WPDevice();
	HRESULT getError();
	void getFileEnumerator(const std::wstring &path, WPDFileEnumerator &enumerator);

	// Return WPD_CONTENT_TYPE_*** GUID
	// See more https://msdn.microsoft.com/en-us/library/windows/hardware/ff597558(v=vs.85).aspx
	bool getObjectType(const std::wstring &path, GUID *guid);

	// If object type is WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT then return WPD_FUNCTIONAL_CATEGORY_*** GUID
	// See more https://msdn.microsoft.com/en-us/library/windows/desktop/dd389028(v=vs.85).aspx
	bool getFunctionalObjectCategoty(const std::wstring &path, GUID *guid);

	bool getFileName(const std::wstring &path, std::wstring *str);

	/* Operations with functional objects */
	bool getSpaceFree(const std::wstring &path, unsigned long long *size);
	bool getSpace(const std::wstring &path, unsigned long long *size);
	bool getFileSystem(const std::wstring &path, std::wstring *str);

	/* Operations with files */
	bool getFileSize(const std::wstring &path, unsigned long long *size);
	bool getFileDateModified(const std::wstring &path, SYSTEMTIME *dateModif);
	bool getFileDateCreated(const std::wstring &path, SYSTEMTIME *dateCreate);
	bool copyFile(const std::wstring &path, const std::wstring &newFile, const FILETIME *dateCreate = nullptr, const FILETIME *dateModif = nullptr);
};