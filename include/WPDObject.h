#pragma once

#include <string>
#include <guiddef.h>
#include <minwinbase.h>


namespace WPD {
    class WPDDevice;

    class WPDObjectIterator;

    class WPDObject {
        friend class WPDDevice;

        const WPDDevice *device;

        std::wstring path;
        mutable std::wstring name;
        mutable GUID objectType;
        mutable GUID functionalObjectCategory;
        mutable uint64_t spaceFree;
        mutable uint64_t space;
        mutable std::wstring fileSystem;

        mutable uint64_t fileSize;
        mutable SYSTEMTIME fileDateModified;
        mutable SYSTEMTIME fileDateCreated;

        mutable bool init;

        void firstInit() const;
        void updateFields() const;

    public:
        WPDObject(const WPDDevice *d, const std::wstring &p);

        WPDObject(const WPDDevice *d, std::wstring &&p);

        std::wstring getPath() const;

        std::wstring getFileName() const;

        bool isFile() const;

        bool isDirectory() const;

        bool isStorage() const;

        GUID getObjectType() const;

        GUID getFunctionalObjectCategory() const;

        uint64_t getSpaceFree() const;

        uint64_t getSpace() const;

        std::wstring getFileSystem() const;

        uint64_t getFileSize() const;

        SYSTEMTIME getFileDateModified() const;

        SYSTEMTIME getFileDateCreated() const;

        WPDObjectIterator begin() const;

        WPDObjectIterator end() const;
    };
}
