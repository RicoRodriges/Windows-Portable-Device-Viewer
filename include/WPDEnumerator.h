#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include <PortableDeviceApi.h>

namespace WPD {
    class WPDEnumerator {
        using PortableDeviceManager = std::unique_ptr<IPortableDeviceManager, void (*)(IPortableDeviceManager *)>;

        PortableDeviceManager deviceManager;
        std::vector<std::wstring> devicePathes;

        static std::wstring getDeviceStringValue(const std::wstring &path,
                                                 const std::function<HRESULT(LPCWSTR, LPWSTR, LPDWORD)> &func);

    protected:
        explicit WPDEnumerator(PortableDeviceManager &&deviceManager);

    public:
        WPDEnumerator();

        void refreshDeviceList();

        int getDeviceCount() const;

        const std::vector<std::wstring> &getDevicePathes() const;

        std::wstring getFriendlyName(const std::wstring &path) const;

        std::wstring getManufacturer(const std::wstring &path) const;

        std::wstring getDescription(const std::wstring &path) const;
    };
}