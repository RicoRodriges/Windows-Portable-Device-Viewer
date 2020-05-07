#include "utils.h"

LPWSTR allocString(const std::wstring &src) {
    const auto memorySize = (src.size() + 1) * sizeof(wchar_t);
    auto mem = static_cast<wchar_t *>(CoTaskMemAlloc(memorySize));
    memcpy_s(mem, memorySize, src.c_str(), memorySize);
    return mem;
}

void copyStrings(const std::vector<std::wstring> &src, LPWSTR *dest) {
    for (int i = 0; i < src.size(); ++i) {
        dest[i] = allocString(src[i]);
    }
}