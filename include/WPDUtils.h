#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <memory>
#include <PortableDeviceTypes.h>

#include "WPDException.h"

namespace WPD {
    namespace Utils {
        bool isFile(const GUID &type);

        bool isDirectory(const GUID &type);

        bool isStorage(const GUID &type, const GUID &category);

        bool getDateParam(IPortableDeviceValues *values, const PROPERTYKEY &key, SYSTEMTIME *date);

        bool getUInt64Param(IPortableDeviceValues *values, const PROPERTYKEY &key, uint64_t *num);

        bool getStringParam(IPortableDeviceValues *values, const PROPERTYKEY &key, std::wstring *str);

        bool getGUIDParam(IPortableDeviceValues *values, const PROPERTYKEY &key, GUID *guid);

        template<class T, const IID &guid>
        std::unique_ptr<T, void (*)(T *)> CreateComInstance() {
            return CreateComInstance<T>([](T **p) {
                return CoCreateInstance(guid,
                                        nullptr,
                                        CLSCTX_INPROC_SERVER,
                                        IID_PPV_ARGS(p));
            });
        }

        template<class T>
        std::unique_ptr<T, void (*)(T *)> CreateComInstance(const std::function<HRESULT(T **)> &creator) {
            T *rawInstance = nullptr;
            auto hr = creator(&rawInstance);
            if (FAILED(hr)) {
                throw WPDException(hr, "Could not initialized new COM object instance");
            }
            return std::unique_ptr<T, void (*)(T *)>(rawInstance, ComDeleter < T > );
        }

        template<class T>
        void ComDeleter(T *p) {
            if (p != nullptr) {
                p->Release();
            }
        }
    }
}
