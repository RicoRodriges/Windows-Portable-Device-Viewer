#pragma once

#include <functional>
#include <string>
#include <memory>

#include <PortableDeviceApi.h>
#include <PortableDevice.h>

namespace WPD {
    class WPDObject;

    class WPDObjectIterator;

    class WPDDevice {
        friend class WPDObject;

        template<class T>
        using ptr = std::unique_ptr<T, void (*)(T *)>;

        // Member declaration order is important!
        ptr<IPortableDevice> device;
        ptr<IPortableDeviceContent> content;
        ptr<IPortableDeviceProperties> properties;
        ptr<IPortableDeviceResources> resources;
        ptr<IPortableDeviceKeyCollection> keys;

        ptr<IStream> openStream(const std::wstring &path);

        static void StreamCopy(const ptr<IStream> &sourceStream, DWORD transferSizeBytes,
                               const std::function<void(const char *, DWORD)> &writer);

        bool getDateParam(const std::wstring &path, SYSTEMTIME *date, const PROPERTYKEY &key) const;

        bool getUInt64Param(const std::wstring &path, ULONGLONG *num, const PROPERTYKEY &key) const;

        bool getStringParam(const std::wstring &path, std::wstring *str, const PROPERTYKEY &key) const;

        bool getGUIDParam(const std::wstring &path, GUID *guid, const PROPERTYKEY &key) const;

        void addKeyToCollection(const PROPERTYKEY &key) const;

        ptr<IPortableDeviceValues> requestValues(const std::wstring &path) const;

        void initObject(const WPDObject *obj) const;

    protected:
        WPDDevice(const std::wstring &path, ptr<IPortableDevice> &&device,
                  ptr<IPortableDeviceKeyCollection> &&keys,
                  ptr<IPortableDeviceValues> &&clientInformation);

    public:
        explicit WPDDevice(const std::wstring &path);

        WPDObjectIterator getFileIterator(const std::wstring &path) const;

        WPDObject getObject(const std::wstring &path) const;

        // Return WPD_CONTENT_TYPE_*** GUID
        // See more https://msdn.microsoft.com/en-us/library/windows/hardware/ff597558(v=vs.85).aspx
        bool getObjectType(const std::wstring &path, GUID *guid) const;

        // If object type is WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT then return WPD_FUNCTIONAL_CATEGORY_*** GUID
        // See more https://msdn.microsoft.com/en-us/library/windows/desktop/dd389028(v=vs.85).aspx
        bool getFunctionalObjectCategory(const std::wstring &path, GUID *guid) const;

        bool getFileName(const std::wstring &path, std::wstring *str) const;

        bool isDirectory(const std::wstring &path);

        bool isStorage(const std::wstring &path);

        bool isFile(const std::wstring &path);

        /* Operations with storage */
        bool getSpaceFree(const std::wstring &path, uint64_t *size) const;

        bool getSpace(const std::wstring &path, uint64_t *size) const;

        bool getFileSystem(const std::wstring &path, std::wstring *str) const;

        /* Operations with files */
        bool getFileSize(const std::wstring &path, uint64_t *size) const;

        bool getFileDateModified(const std::wstring &path, SYSTEMTIME *dateModif) const;

        bool getFileDateCreated(const std::wstring &path, SYSTEMTIME *dateCreate) const;

        // Save file to PC
        bool downloadFile(const std::wstring &path, const std::wstring &newFile, const FILETIME *dateCreate = nullptr,
                          const FILETIME *dateModif = nullptr);
        bool downloadFile(const std::wstring &path, const std::wstring &newFile, const SYSTEMTIME &dateCreate,
                          const SYSTEMTIME &dateModif);

        // Read file to buffer. It's not an ASCII string
        std::string readFile(const std::wstring &path);

        // Read batchSize bytes of file and call handler
        void readFile(const std::wstring &path, uint32_t batchSize,
                      const std::function<void(const char *buf, uint32_t len)> &handler);
    };
}