#include <cassert>

#include "WPDDevice.h"
#include "WPDObjectIterator.h"
#include "WPDException.h"
#include "WPDObject.h"
#include "WPDUtils.h"

#define CHECK_ARG(arg) { if (arg == nullptr) throw std::invalid_argument(#arg" pointer is null"); }

WPD::WPDDevice::WPDDevice(const std::wstring &path) : WPDDevice(path,
                                                                Utils::CreateComInstance<IPortableDevice, CLSID_PortableDeviceFTM>(),
                                                                Utils::CreateComInstance<IPortableDeviceKeyCollection, CLSID_PortableDeviceKeyCollection>(),
                                                                Utils::CreateComInstance<IPortableDeviceValues, CLSID_PortableDeviceValues>()) {}

WPD::WPDDevice::WPDDevice(const std::wstring &path, ptr<IPortableDevice> &&d, ptr<IPortableDeviceKeyCollection> &&keys,
                          ptr<IPortableDeviceValues> &&clientInformation) :
        device(std::move(d)), keys(std::move(keys)), content(nullptr, Utils::ComDeleter<IPortableDeviceContent>),
        properties(nullptr, Utils::ComDeleter<IPortableDeviceProperties>), resources(nullptr, Utils::ComDeleter<IPortableDeviceResources>) {

    clientInformation->SetStringValue(WPD_CLIENT_NAME, L"WPD Viewer");
    clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1);
    clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 0);
    clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 1);
    clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
    clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SHARE_MODE,
                                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);

    auto hr = device->Open(path.c_str(), clientInformation.get());
    if (FAILED(hr)) throw WPDException(hr, "Couldn't open a connection between the application and the device");

    content = Utils::CreateComInstance<IPortableDeviceContent>([this](IPortableDeviceContent **p) {
        return device->Content(p);
    });

    properties = Utils::CreateComInstance<IPortableDeviceProperties>([this](IPortableDeviceProperties **p) {
        return content->Properties(p);
    });

    resources = Utils::CreateComInstance<IPortableDeviceResources>([this](IPortableDeviceResources **p) {
        return content->Transfer(p);
    });
}

WPD::WPDObjectIterator WPD::WPDDevice::getFileIterator(const std::wstring &path) const {
    auto enumObjectIDs = Utils::CreateComInstance<IEnumPortableDeviceObjectIDs>([this, &path](IEnumPortableDeviceObjectIDs **p) {
        return content->EnumObjects(0,
                                    path.empty() ? WPD_DEVICE_OBJECT_ID : path.c_str(),
                                    nullptr,
                                    p);
    });
    return WPDObjectIterator{std::move(enumObjectIDs), this, path};
}

bool WPD::WPDDevice::getDateParam(const std::wstring &path, SYSTEMTIME *date, const PROPERTYKEY &key) const {
    CHECK_ARG(date);

    keys->Clear();
    addKeyToCollection(key);

    auto objectProperties = requestValues(path);
    return WPD::Utils::getDateParam(objectProperties.get(), key, date);
}

bool WPD::WPDDevice::getUInt64Param(const std::wstring &path, uint64_t *num, const PROPERTYKEY &key) const {
    CHECK_ARG(num);

    keys->Clear();
    addKeyToCollection(key);

    auto objectProperties = requestValues(path);
    return WPD::Utils::getUInt64Param(objectProperties.get(), key, num);
}

bool WPD::WPDDevice::getStringParam(const std::wstring &path, std::wstring *str, const PROPERTYKEY &key) const {
    CHECK_ARG(str);

    keys->Clear();
    addKeyToCollection(key);

    auto objectProperties = requestValues(path);
    return WPD::Utils::getStringParam(objectProperties.get(), key, str);
}

bool WPD::WPDDevice::getGUIDParam(const std::wstring &path, GUID *guid, const PROPERTYKEY &key) const {
    CHECK_ARG(guid);

    keys->Clear();
    addKeyToCollection(key);

    auto objectProperties = requestValues(path);
    return WPD::Utils::getGUIDParam(objectProperties.get(), key, guid);
}

bool WPD::WPDDevice::getSpaceFree(const std::wstring &path, unsigned long long *size) const {
    return getUInt64Param(path, size, WPD_STORAGE_FREE_SPACE_IN_BYTES);
}

bool WPD::WPDDevice::getSpace(const std::wstring &path, unsigned long long *size) const {
    return getUInt64Param(path, size, WPD_STORAGE_CAPACITY);
}

bool WPD::WPDDevice::getFileSystem(const std::wstring &path, std::wstring *str) const {
    return getStringParam(path, str, WPD_STORAGE_FILE_SYSTEM_TYPE);
}

bool WPD::WPDDevice::getFileSize(const std::wstring &path, unsigned long long *size) const {
    return getUInt64Param(path, size, WPD_OBJECT_SIZE);
}

bool WPD::WPDDevice::getFileName(const std::wstring &path, std::wstring *str) const {
    CHECK_ARG(str);

    keys->Clear();
    addKeyToCollection(WPD_OBJECT_ORIGINAL_FILE_NAME);
    addKeyToCollection(WPD_OBJECT_NAME);

    auto response = requestValues(path);

    bool result = Utils::getStringParam(response.get(), WPD_OBJECT_ORIGINAL_FILE_NAME, str);
    if (!result) {
        result = Utils::getStringParam(response.get(), WPD_OBJECT_NAME, str);
    }
    return result;
}

bool WPD::WPDDevice::getFileDateModified(const std::wstring &path, SYSTEMTIME *dateModif) const {
    return getDateParam(path, dateModif, WPD_OBJECT_DATE_MODIFIED);
}

bool WPD::WPDDevice::getFileDateCreated(const std::wstring &path, SYSTEMTIME *dateCreate) const {
    return getDateParam(path, dateCreate, WPD_OBJECT_DATE_CREATED);
}

bool WPD::WPDDevice::getObjectType(const std::wstring &path, GUID *guid) const {
    return getGUIDParam(path, guid, WPD_OBJECT_CONTENT_TYPE);
}

bool WPD::WPDDevice::getFunctionalObjectCategory(const std::wstring &path, GUID *guid) const {
    return getGUIDParam(path, guid, WPD_FUNCTIONAL_OBJECT_CATEGORY);
}

bool WPD::WPDDevice::downloadFile(const std::wstring &path, const std::wstring &newFile, const FILETIME *dateCreate,
                                  const FILETIME *dateModif) {
    auto objectDataStream = openStream(path);

    std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&CloseHandle)> finalFile{
            CreateFileW(newFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                        nullptr),
            CloseHandle
    };
    if (finalFile.get() == INVALID_HANDLE_VALUE) {
        auto hr = GetLastError();
        throw WPDException(hr, "Couldn't create a file");
    }

    StreamCopy(objectDataStream, 1024 * 1024, [&finalFile](const char *buf, DWORD size) {
        DWORD bytesWritten = 0;
        do {
            if (!WriteFile(finalFile.get(), buf + bytesWritten, size - bytesWritten, &bytesWritten, nullptr)) {
                throw WPDException(GetLastError(), "Couldn't write to file");
            }
        } while (size > bytesWritten);
    });

    if (dateCreate || dateModif)
        SetFileTime(finalFile.get(), dateCreate, dateModif, dateModif);

    return true;
}

bool WPD::WPDDevice::downloadFile(const std::wstring &path, const std::wstring &newFile, const SYSTEMTIME &dateCreate,
                                  const SYSTEMTIME &dateModif) {
    bool result = true;

    FILETIME fcreate, fmodif;
    {
        FILETIME temp;
        result = result && SystemTimeToFileTime(&dateCreate, &temp);
        result = result && LocalFileTimeToFileTime(&temp, &fcreate);
    }
    {
        FILETIME temp;
        result = result && SystemTimeToFileTime(&dateModif, &temp);
        result = result && LocalFileTimeToFileTime(&temp, &fmodif);
    }
    if (!result) {
        throw WPDException(0, "Date is invalid");
    }

    return downloadFile(path, newFile, &fcreate, &fmodif);
}

std::string WPD::WPDDevice::readFile(const std::wstring &path) {
    std::string buf;

    uint64_t size = 0;
    if (getFileSize(path, &size)) {
        buf.reserve(size);
    }

    const uint32_t batchSize = 1024 * 1024;
    readFile(path, batchSize, [&buf](const char *b, uint32_t size) {
        buf.append(b, size);
    });
    return std::move(buf);
}

void WPD::WPDDevice::readFile(const std::wstring &path, uint32_t batchSize,
                              const std::function<void(const char *, uint32_t)> &handler) {
    auto objectDataStream = openStream(path);
    StreamCopy(objectDataStream, batchSize, handler);
}

WPD::WPDDevice::ptr<IStream> WPD::WPDDevice::openStream(const std::wstring &path) {
    return Utils::CreateComInstance<IStream>([this, &path](IStream **p) {
        DWORD optimalTransferSizeBytes = 0;
        return resources->GetStream(path.c_str(), WPD_RESOURCE_DEFAULT, STGM_READ, &optimalTransferSizeBytes, p);
    });
}

void WPD::WPDDevice::StreamCopy(const ptr<IStream> &sourceStream, DWORD transferSizeBytes,
                                const std::function<void(const char *, DWORD)> &writer) {
    DWORD bytesRead;
    HRESULT hr;

    auto objectData = std::make_unique<char[]>(transferSizeBytes);
    while (true) {
        hr = sourceStream->Read(objectData.get(), transferSizeBytes, &bytesRead);
        if (FAILED(hr)) throw WPDException(hr, "Couldn't read a stream");
        if (bytesRead == 0) break;

        writer(objectData.get(), bytesRead);
    }
}

bool WPD::WPDDevice::isDirectory(const std::wstring &path) {
    GUID guid;
    return getObjectType(path, &guid) && Utils::isDirectory(guid);
}

bool WPD::WPDDevice::isFile(const std::wstring &path) {
    GUID guid;
    return getObjectType(path, &guid) && Utils::isFile(guid);
}

bool WPD::WPDDevice::isStorage(const std::wstring &path) {
    GUID guid;
    GUID category;
    return getObjectType(path, &guid) && IsEqualGUID(guid, WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT) &&
           getFunctionalObjectCategory(path, &category) && Utils::isStorage(guid, category);
}

WPD::WPDObject WPD::WPDDevice::getObject(const std::wstring &path) const {
    return WPDObject{this, path};
}

void WPD::WPDDevice::initObject(const WPD::WPDObject *obj) const {
    CHECK_ARG(obj);
    assert(!obj->path.empty());

    keys->Clear();
    addKeyToCollection(WPD_OBJECT_ORIGINAL_FILE_NAME);
    addKeyToCollection(WPD_OBJECT_NAME);

    addKeyToCollection(WPD_OBJECT_CONTENT_TYPE);
    addKeyToCollection(WPD_FUNCTIONAL_OBJECT_CATEGORY);

    addKeyToCollection(WPD_STORAGE_FREE_SPACE_IN_BYTES);
    addKeyToCollection(WPD_STORAGE_CAPACITY);
    addKeyToCollection(WPD_STORAGE_FILE_SYSTEM_TYPE);

    addKeyToCollection(WPD_OBJECT_SIZE);

    addKeyToCollection(WPD_OBJECT_DATE_MODIFIED);
    addKeyToCollection(WPD_OBJECT_DATE_CREATED);

    auto response = requestValues(obj->path);

    if (!Utils::getStringParam(response.get(), WPD_OBJECT_ORIGINAL_FILE_NAME, &obj->name)) {
        if (!Utils::getStringParam(response.get(), WPD_OBJECT_NAME, &obj->name)) {
            obj->name.clear();
        }
    }

    if (!Utils::getGUIDParam(response.get(), WPD_OBJECT_CONTENT_TYPE, &obj->objectType)) {
        ZeroMemory(&obj->objectType, sizeof(obj->objectType));
    }
    if (!Utils::getGUIDParam(response.get(), WPD_FUNCTIONAL_OBJECT_CATEGORY, &obj->functionalObjectCategory)) {
        ZeroMemory(&obj->functionalObjectCategory, sizeof(obj->functionalObjectCategory));
    }

    if (!Utils::getUInt64Param(response.get(), WPD_STORAGE_FREE_SPACE_IN_BYTES, &obj->spaceFree)) {
        obj->spaceFree = 0;
    }
    if (!Utils::getUInt64Param(response.get(), WPD_STORAGE_CAPACITY, &obj->space)) {
        obj->space = 0;
    }
    if (!Utils::getStringParam(response.get(), WPD_STORAGE_FILE_SYSTEM_TYPE, &obj->fileSystem)) {
        obj->fileSystem.clear();
    }

    if (!Utils::getUInt64Param(response.get(), WPD_OBJECT_SIZE, &obj->fileSize)) {
        obj->fileSize = 0;
    }

    if (!Utils::getDateParam(response.get(), WPD_OBJECT_DATE_MODIFIED, &obj->fileDateModified)) {
        ZeroMemory(&obj->fileDateModified, sizeof(obj->fileDateModified));
    }
    if (!Utils::getDateParam(response.get(), WPD_OBJECT_DATE_CREATED, &obj->fileDateCreated)) {
        ZeroMemory(&obj->fileDateCreated, sizeof(obj->fileDateCreated));
    }
}

void WPD::WPDDevice::addKeyToCollection(const PROPERTYKEY &key) const {
    auto hr = keys->Add(key);
    if (FAILED(hr)) throw WPDException(hr, "Couldn't add a property key to the collection");
}

WPD::WPDDevice::ptr<IPortableDeviceValues> WPD::WPDDevice::requestValues(const std::wstring &path) const {
    return Utils::CreateComInstance<IPortableDeviceValues>([this, &path](IPortableDeviceValues **p) {
        // May return S_FALSE if one or more property values could not be retrieved
        properties->GetValues(path.c_str(), keys.get(), p);
        return S_OK;
    });
}
