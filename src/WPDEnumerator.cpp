#include "WPDUtils.h"

#include "WPDEnumerator.h"

WPD::WPDEnumerator::WPDEnumerator() : WPDEnumerator(
        Utils::CreateComInstance<IPortableDeviceManager, CLSID_PortableDeviceManager>()) {}

WPD::WPDEnumerator::WPDEnumerator(PortableDeviceManager &&deviceManager) : deviceManager(std::move(deviceManager)) {
    refreshDeviceList();
}

int WPD::WPDEnumerator::getDeviceCount() const { return devicePathes.size(); }

void WPD::WPDEnumerator::refreshDeviceList() {
    devicePathes.clear();
    deviceManager->RefreshDeviceList();

    DWORD deviceCount = 0;
    HRESULT hr = deviceManager->GetDevices(nullptr, &deviceCount);
    if (SUCCEEDED(hr) && deviceCount > 0) {

        DWORD retrievedDeviceIDCount = deviceCount;
        auto deleter = [&retrievedDeviceIDCount](wchar_t **v) {
            for (DWORD index = 0; index < retrievedDeviceIDCount; index++)
                CoTaskMemFree(v[index]);
            delete[] v;
        };
        std::unique_ptr<wchar_t *[], decltype(deleter)> pnpDeviceIDs{new wchar_t *[deviceCount], deleter};
        hr = deviceManager->GetDevices(pnpDeviceIDs.get(), &retrievedDeviceIDCount);
        if (FAILED(hr))
            return;

        devicePathes.reserve(retrievedDeviceIDCount);
        for (int i = 0; i < retrievedDeviceIDCount; ++i) {
            devicePathes.emplace_back(pnpDeviceIDs[i]);
        }

    }
}

const std::vector<std::wstring> &WPD::WPDEnumerator::getDevicePathes() const {
    return devicePathes;
}

std::wstring WPD::WPDEnumerator::getDeviceStringValue(const std::wstring &path,
                                                      const std::function<HRESULT(LPCWSTR, LPWSTR,
                                                                                  LPDWORD)> &func) {
    DWORD length = 0;
    HRESULT hr = func(path.c_str(), 0, &length);
    if (hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
        throw WPDException(hr, "Device was not found.");
    } else if (FAILED(hr) || length == 0) {
        return std::wstring{};
    }

    auto temp = std::make_unique<wchar_t[]>(length);

    hr = func(path.c_str(), temp.get(), &length);
    if (FAILED(hr)) {
        return std::wstring{};
    }

    return std::wstring{temp.get(), length};
}

std::wstring WPD::WPDEnumerator::getFriendlyName(const std::wstring &path) const {
    return getDeviceStringValue(path,
                                [this](auto path, auto res, auto len) {
                                    return deviceManager->GetDeviceFriendlyName(path, res, len);
                                }
    );
}

std::wstring WPD::WPDEnumerator::getManufacturer(const std::wstring &path) const {
    return getDeviceStringValue(path,
                                [this](auto path, auto res, auto len) {
                                    return deviceManager->GetDeviceManufacturer(path, res, len);
                                }
    );
}

std::wstring WPD::WPDEnumerator::getDescription(const std::wstring &path) const {
    return getDeviceStringValue(path,
                                [this](auto path, auto res, auto len) {
                                    return deviceManager->GetDeviceDescription(path, res, len);
                                }
    );
}
