#pragma once

#include <stdexcept>
#include <PortableDeviceApi.h>

// Unfortunately Fakeit cannot mock __stdcall methods
// It generates implementation for __stdcall method which calls __cdecl virtual methods
#define PROXY_STDCALL(resultDef, name, paramsDef, params, result) \
    resultDef __stdcall name paramsDef override { \
        return I##name params; \
    } \
    virtual resultDef I##name paramsDef { throw std::runtime_error{#name" is not implemented"}; }

class PortableDeviceManagerWrapper : public IPortableDeviceManager {
public:
    PROXY_STDCALL(HRESULT, GetDevices, (LPWSTR * a, DWORD * b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, RefreshDeviceList, (), (), S_OK);

    PROXY_STDCALL(HRESULT, GetDeviceFriendlyName, (LPCWSTR a, WCHAR * b, DWORD * c), (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, GetDeviceDescription, (LPCWSTR a, WCHAR * b, DWORD * c), (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, GetDeviceManufacturer, (LPCWSTR a, WCHAR * b, DWORD * c), (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, GetDeviceProperty, (LPCWSTR a, LPCWSTR b, BYTE * c, DWORD * d, DWORD * e), (a, b, c, d, e),
                  S_OK);

    PROXY_STDCALL(HRESULT, GetPrivateDevices, (LPWSTR * a, DWORD * b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};

class PortableDeviceWrapper : public IPortableDevice {
public:
    PROXY_STDCALL(HRESULT, Open, (LPCWSTR a, IPortableDeviceValues * b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SendCommand, (DWORD a, IPortableDeviceValues * b, IPortableDeviceValues * *c), (a, b, c),
                  S_OK);

    PROXY_STDCALL(HRESULT, Content, (IPortableDeviceContent * *a), (a), S_OK);

    PROXY_STDCALL(HRESULT, Capabilities, (IPortableDeviceCapabilities * *a), (a), S_OK);

    PROXY_STDCALL(HRESULT, Cancel, (), (), S_OK);

    PROXY_STDCALL(HRESULT, Close, (), (), S_OK);

    PROXY_STDCALL(HRESULT, Advise, (DWORD a, IPortableDeviceEventCallback * b, IPortableDeviceValues * c, LPWSTR * d),
                  (a, b, c, d), S_OK);

    PROXY_STDCALL(HRESULT, Unadvise, (LPCWSTR a), (a), S_OK);

    PROXY_STDCALL(HRESULT, GetPnPDeviceID, (LPWSTR * a), (a), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};

class PortableDeviceKeyCollectionWrapper : public IPortableDeviceKeyCollection {
public:
    PROXY_STDCALL(HRESULT, GetCount, (DWORD * a), (a), S_OK);

    PROXY_STDCALL(HRESULT, GetAt, (DWORD a, PROPERTYKEY * b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, Add, (const PROPERTYKEY &a), (a), S_OK);

    PROXY_STDCALL(HRESULT, Clear, (), (), S_OK);

    PROXY_STDCALL(HRESULT, RemoveAt, (DWORD a), (a), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};

class PortableDeviceContentWrapper : public IPortableDeviceContent {
public:
    PROXY_STDCALL(HRESULT, EnumObjects,
                  (DWORD a, LPCWSTR b, IPortableDeviceValues * c, IEnumPortableDeviceObjectIDs * *d), (a, b, c, d),
                  S_OK);

    PROXY_STDCALL(HRESULT, Properties, (IPortableDeviceProperties * *a), (a), S_OK);

    PROXY_STDCALL(HRESULT, Transfer, (IPortableDeviceResources * *a), (a), S_OK);

    PROXY_STDCALL(HRESULT, CreateObjectWithPropertiesOnly, (IPortableDeviceValues * a, LPWSTR * b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, CreateObjectWithPropertiesAndData,
                  (IPortableDeviceValues * a, IStream * *b, DWORD * c, LPWSTR * d), (a, b, c, d), S_OK);

    PROXY_STDCALL(HRESULT, Delete,
                  (DWORD a, IPortableDevicePropVariantCollection * b, IPortableDevicePropVariantCollection * *c),
                  (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, GetObjectIDsFromPersistentUniqueIDs,
                  (IPortableDevicePropVariantCollection * a, IPortableDevicePropVariantCollection * *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, Cancel, (), (), S_OK);

    PROXY_STDCALL(HRESULT, Move,
                  (IPortableDevicePropVariantCollection * a, LPCWSTR b, IPortableDevicePropVariantCollection * *c),
                  (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, Copy,
                  (IPortableDevicePropVariantCollection * a, LPCWSTR b, IPortableDevicePropVariantCollection * *c),
                  (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};

class PortableDeviceResourcesWrapper : public IPortableDeviceResources {
public:
    PROXY_STDCALL(HRESULT, GetSupportedResources, (LPCWSTR a, IPortableDeviceKeyCollection * *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetResourceAttributes, (LPCWSTR a, const PROPERTYKEY &b, IPortableDeviceValues **c),
                  (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, GetStream, (LPCWSTR a, const PROPERTYKEY &b, DWORD c, DWORD * d, IStream * *e),
                  (a, b, c, d, e), S_OK);

    PROXY_STDCALL(HRESULT, Delete, (LPCWSTR a, IPortableDeviceKeyCollection * b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, Cancel, (), (), S_OK);

    PROXY_STDCALL(HRESULT, CreateResource, (IPortableDeviceValues * a, IStream * *b, DWORD * c, LPWSTR * d),
                  (a, b, c, d), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};

class PortableDevicePropertiesWrapper : public IPortableDeviceProperties {
public:
    PROXY_STDCALL(HRESULT, GetSupportedProperties, (LPCWSTR a, IPortableDeviceKeyCollection * *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetPropertyAttributes, (LPCWSTR a, const PROPERTYKEY &b, IPortableDeviceValues **c),
                  (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, GetValues, (LPCWSTR a, IPortableDeviceKeyCollection * b, IPortableDeviceValues * *c),
                  (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, SetValues, (LPCWSTR a, IPortableDeviceValues * b, IPortableDeviceValues * *c), (a, b, c),
                  S_OK);

    PROXY_STDCALL(HRESULT, Delete, (LPCWSTR a, IPortableDeviceKeyCollection * b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, Cancel, (), (), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};

class PortableDeviceValuesWrapper : public IPortableDeviceValues {
public:
    PROXY_STDCALL(HRESULT, GetCount, (DWORD * a), (a), S_OK);

    PROXY_STDCALL(HRESULT, GetAt, (DWORD a, PROPERTYKEY * b, PROPVARIANT * c), (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, SetValue, (const PROPERTYKEY &a, const PROPVARIANT *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetValue, (const PROPERTYKEY &a, PROPVARIANT *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetStringValue, (const PROPERTYKEY &a, LPCWSTR b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetStringValue, (const PROPERTYKEY &a, LPWSTR *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetUnsignedIntegerValue, (const PROPERTYKEY &a, ULONG b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetUnsignedIntegerValue, (const PROPERTYKEY &a, ULONG *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetSignedIntegerValue, (const PROPERTYKEY &a, LONG b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetSignedIntegerValue, (const PROPERTYKEY &a, LONG *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetUnsignedLargeIntegerValue, (const PROPERTYKEY &a, ULONGLONG b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetUnsignedLargeIntegerValue, (const PROPERTYKEY &a, ULONGLONG *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetSignedLargeIntegerValue, (const PROPERTYKEY &a, LONGLONG b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetSignedLargeIntegerValue, (const PROPERTYKEY &a, LONGLONG *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetFloatValue, (const PROPERTYKEY &a, FLOAT b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetFloatValue, (const PROPERTYKEY &a, FLOAT *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetErrorValue, (const PROPERTYKEY &a, HRESULT b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetErrorValue, (const PROPERTYKEY &a, HRESULT *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetKeyValue, (const PROPERTYKEY &a, const PROPERTYKEY &b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetKeyValue, (const PROPERTYKEY &a, PROPERTYKEY *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetBoolValue, (const PROPERTYKEY &a, BOOL b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetBoolValue, (const PROPERTYKEY &a, BOOL *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetIUnknownValue, (const PROPERTYKEY &a, IUnknown *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetIUnknownValue, (const PROPERTYKEY &a, IUnknown **b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetGuidValue, (const PROPERTYKEY &a, const GUID &b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetGuidValue, (const PROPERTYKEY &a, GUID *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetBufferValue, (const PROPERTYKEY &a, BYTE *b, DWORD c), (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, GetBufferValue, (const PROPERTYKEY &a, BYTE **b, DWORD *c), (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, SetIPortableDeviceValuesValue, (const PROPERTYKEY &a, IPortableDeviceValues *b), (a, b),
                  S_OK);

    PROXY_STDCALL(HRESULT, GetIPortableDeviceValuesValue, (const PROPERTYKEY &a, IPortableDeviceValues **b), (a, b),
                  S_OK);

    PROXY_STDCALL(HRESULT, SetIPortableDevicePropVariantCollectionValue,
                  (const PROPERTYKEY &a, IPortableDevicePropVariantCollection *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetIPortableDevicePropVariantCollectionValue,
                  (const PROPERTYKEY &a, IPortableDevicePropVariantCollection **b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetIPortableDeviceKeyCollectionValue,
                  (const PROPERTYKEY &a, IPortableDeviceKeyCollection *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetIPortableDeviceKeyCollectionValue,
                  (const PROPERTYKEY &a, IPortableDeviceKeyCollection **b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, SetIPortableDeviceValuesCollectionValue,
                  (const PROPERTYKEY &a, IPortableDeviceValuesCollection *b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, GetIPortableDeviceValuesCollectionValue,
                  (const PROPERTYKEY &a, IPortableDeviceValuesCollection **b), (a, b), S_OK);

    PROXY_STDCALL(HRESULT, RemoveValue, (const PROPERTYKEY &a), (a), S_OK);

    PROXY_STDCALL(HRESULT, CopyValuesFromPropertyStore, (IPropertyStore * a), (a), S_OK);

    PROXY_STDCALL(HRESULT, CopyValuesToPropertyStore, (IPropertyStore * a), (a), S_OK);

    PROXY_STDCALL(HRESULT, Clear, (), (), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};

class PortableDeviceEnumWrapper : public IEnumPortableDeviceObjectIDs {
public:
    PROXY_STDCALL(HRESULT, Next, (ULONG a, LPWSTR *b, ULONG *c), (a, b, c), S_OK);

    PROXY_STDCALL(HRESULT, Skip, (ULONG a), (a), S_OK);

    PROXY_STDCALL(HRESULT, Reset, (), (), S_OK);

    PROXY_STDCALL(HRESULT, Clone, (IEnumPortableDeviceObjectIDs **a), (a), S_OK);

    PROXY_STDCALL(HRESULT, Cancel, (), (), S_OK);

    PROXY_STDCALL(HRESULT, QueryInterface, (const IID &a, void **b), (a, b), S_OK);

    PROXY_STDCALL(ULONG, AddRef, (), (), 0);

    PROXY_STDCALL(ULONG, Release, (), (), 0);
};