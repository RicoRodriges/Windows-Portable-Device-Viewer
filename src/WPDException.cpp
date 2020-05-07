#include <comdef.h>

#include "WPDException.h"

WPD::WPDException::WPDException(HRESULT code, const char *str) : std::runtime_error(str), c(code) {}

WPD::WPDException::WPDException(HRESULT code, const std::string &str) : std::runtime_error(str), c(code) {}

HRESULT WPD::WPDException::code() const { return c; }

#ifdef UNICODE

std::wstring WPD::WPDException::explain() const {
    _com_error err(c);
    return std::wstring{static_cast<const wchar_t *>(err.ErrorMessage())};
}
#else

std::string WPD::WPDException::explain() const {
    _com_error err(c);
    return std::string{static_cast<const char *>(err.ErrorMessage())};
}

#endif
