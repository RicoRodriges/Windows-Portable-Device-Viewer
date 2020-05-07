#pragma once

#include <vector>
#include <combaseapi.h>

#include "WPDUtils.h"

template <class T>
using ptr = std::unique_ptr<T, void (*)(T *)>;

template<class T>
ptr<T> mockToPtr(T &m) {
    return ptr<T>{&m, WPD::Utils::ComDeleter<T>};
}

LPWSTR allocString(const std::wstring &src);

void copyStrings(const std::vector<std::wstring> &src, LPWSTR *dest);
