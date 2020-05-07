#pragma once

#include <stdexcept>
#include <winerror.h>
#include <string>

namespace WPD {
    class WPDException : public std::runtime_error {
        HRESULT c;
    public:
        WPDException(HRESULT code, const char *str);
        WPDException(HRESULT code, const std::string &str);

        HRESULT code() const;

#ifdef UNICODE

        std::wstring explain() const;

#else

        std::string explain() const;

#endif
    };
}