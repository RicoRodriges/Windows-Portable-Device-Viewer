#pragma once

#include <vector>
#include <string>
#include <memory>

#include <PortableDeviceApi.h>

namespace WPD {
    class WPDDevice;

    class WPDObjectIterator {
        static const int batch = 10;

        using EnumPortableDeviceObjectIDs = std::unique_ptr<IEnumPortableDeviceObjectIDs, void (*)(
                IEnumPortableDeviceObjectIDs *)>;

        EnumPortableDeviceObjectIDs iter;
        const WPDDevice *device;
        bool ended;

        const std::wstring path;
        std::vector<std::wstring> pathes;
        int offset;
        int pos;

        void nextFiles();

    public:

        static WPDObjectIterator endIterator(const WPDDevice *device, const std::wstring &path);

        WPDObjectIterator(EnumPortableDeviceObjectIDs &&p, const WPDDevice *device, std::wstring path);

        std::wstring operator*() const;

        void operator++();

        bool operator==(const WPDObjectIterator &it) const;

        bool operator!=(const WPDObjectIterator &it) const;

        void skip(int num);

        void reset();

    };
}
