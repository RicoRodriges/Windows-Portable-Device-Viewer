#include "WPDUtils.h"

#include <PortableDevice.h>

// TODO: I'm not sure
bool WPD::Utils::isFile(const GUID &type) {
    return !IsEqualGUID(type, WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT) &&
           !IsEqualGUID(type, WPD_CONTENT_TYPE_FOLDER);
}

bool WPD::Utils::isDirectory(const GUID &type) {
    return IsEqualGUID(type, WPD_CONTENT_TYPE_FOLDER);
}

bool WPD::Utils::isStorage(const GUID &type, const GUID &category) {
    return IsEqualGUID(type, WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT) &&
           IsEqualGUID(category, WPD_FUNCTIONAL_CATEGORY_STORAGE);
}

bool WPD::Utils::getDateParam(IPortableDeviceValues *values, const PROPERTYKEY &key, SYSTEMTIME *date) {
    PROPVARIANT temp;

    HRESULT hr = values->GetValue(key, &temp);
    if (FAILED(hr)) return false;
    std::unique_ptr<PROPVARIANT, decltype(&PropVariantClear)> variant{&temp, PropVariantClear};

    return VariantTimeToSystemTime(variant->date, date);
}

bool WPD::Utils::getUInt64Param(IPortableDeviceValues *values, const PROPERTYKEY &key, uint64_t *num) {
    return SUCCEEDED(values->GetUnsignedLargeIntegerValue(key, num));
}

bool WPD::Utils::getStringParam(IPortableDeviceValues *values, const PROPERTYKEY &key, std::wstring *str) {
    wchar_t *temp = nullptr;

    HRESULT hr = values->GetStringValue(key, &temp);
    if (FAILED(hr)) return false;
    std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> result{temp, CoTaskMemFree};

    str->assign(result.get());

    return true;
}

bool WPD::Utils::getGUIDParam(IPortableDeviceValues *values, const PROPERTYKEY &key, GUID *guid) {
    return SUCCEEDED(values->GetGuidValue(key, guid));
}
