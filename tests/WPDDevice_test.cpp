#include "shared_tests.h"

#include <propvarutil.h>

#include "WPDDevice.h"
#include "WPDObject.h"
#include "WPDObjectIterator.h"

using namespace fakeit;

class WPDDevice : public WPD::WPDDevice {
public:
    WPDDevice(const std::wstring &path, ::ptr<IPortableDevice> &&d, ::ptr<IPortableDeviceKeyCollection> &&dkc,
              ::ptr<IPortableDeviceValues> &&v) :
            WPD::WPDDevice(path, std::move(d), std::move(dkc), std::move(v)) {}
};

void mockConstructor(Mock<PortableDeviceWrapper> &deviceMock,
                     Mock<PortableDeviceContentWrapper> &contentMock,
                     Mock<PortableDeviceResourcesWrapper> &resourceMock,
                     Mock<PortableDevicePropertiesWrapper> &propertiesMock,
                     Mock<PortableDeviceValuesWrapper> &clientInfoMock,
                     const std::wstring &expectedPath) {
    Fake(Method(clientInfoMock, ISetStringValue));
    Fake(Method(clientInfoMock, ISetUnsignedIntegerValue));

    When(Method(deviceMock, IOpen).Using(EqSTR<LPCWSTR>(expectedPath), _)).Return(S_OK);

    IPortableDeviceContent &content = contentMock.get();
    When(Method(deviceMock, IContent)).Do([&content](IPortableDeviceContent **p) {
        *p = &content;
        return S_OK;
    });

    IPortableDeviceProperties &properties = propertiesMock.get();
    When(Method(contentMock, IProperties)).Do([&properties](IPortableDeviceProperties **p) {
        *p = &properties;
        return S_OK;
    });

    IPortableDeviceResources &resources = resourceMock.get();
    When(Method(contentMock, ITransfer)).Do([&resources](IPortableDeviceResources **p) {
        *p = &resources;
        return S_OK;
    });

    Fake(Method(deviceMock, IRelease));
    Fake(Method(contentMock, IRelease));
    Fake(Method(resourceMock, IRelease));
    Fake(Method(propertiesMock, IRelease));
    Fake(Method(clientInfoMock, IRelease));
}

TEST_CASE("WPDDevice & WPDObject classes tests") {

    SECTION("Deserialization tests") {
        PortableDeviceWrapper deviceStub;
        PortableDeviceContentWrapper contentStub;
        PortableDeviceKeyCollectionWrapper keyCollectionStub;
        PortableDeviceResourcesWrapper resourceStub;
        PortableDevicePropertiesWrapper propertiesStub;
        PortableDeviceValuesWrapper valuesStub;
        PortableDeviceValuesWrapper clientInfoStub;

        Mock<PortableDeviceWrapper> deviceMock{deviceStub};
        Mock<PortableDeviceContentWrapper> contentMock{contentStub};
        Mock<PortableDeviceKeyCollectionWrapper> deviceKeyMock{keyCollectionStub};
        Mock<PortableDeviceResourcesWrapper> resourceMock{resourceStub};
        Mock<PortableDevicePropertiesWrapper> propertiesMock{propertiesStub};
        Mock<PortableDeviceValuesWrapper> valuesMock{valuesStub};
        Mock<PortableDeviceValuesWrapper> clientInfoMock{clientInfoStub};

        const std::wstring devicePath = L"Some device ID";
        const std::wstring objectPath = L"Some object path";
        const std::wstring expectedObjectName = L"Some object name";
        const int expectedObjectSize = 123;
        GUID expectedObjectCategory = WPD_FUNCTIONAL_CATEGORY_STORAGE;
        SYSTEMTIME expectedCreationDate;
        GetSystemTime(&expectedCreationDate);

        mockConstructor(deviceMock, contentMock, resourceMock, propertiesMock, clientInfoMock, devicePath);

        Fake(Method(deviceKeyMock, IClear));
        Fake(Method(deviceKeyMock, IAdd));
        Fake(Method(deviceKeyMock, IRelease));
        Fake(Method(valuesMock, IRelease));


        IPortableDeviceValues &v = valuesMock.get();
        When(Method(propertiesMock, IGetValues).Using(EqSTR<LPCWSTR>(objectPath), _, _)).AlwaysDo(
                [&v](auto, auto, auto val) {
                    *val = &v;
                    return S_OK;
                });

        When(Method(valuesMock, IGetStringValue).Using(Eq(WPD_OBJECT_NAME), _)).AlwaysDo(
                [&expectedObjectName](auto k, auto s) {
                    *s = allocString(expectedObjectName);
                    return S_OK;
                });
        When(Method(valuesMock, IGetStringValue).Using(Ne(WPD_OBJECT_NAME), _)).AlwaysReturn(~S_OK);


        When(Method(valuesMock, IGetUnsignedLargeIntegerValue).Using(Eq(WPD_OBJECT_SIZE), _)).AlwaysDo(
                [expectedObjectSize](auto k, auto s) {
                    *s = expectedObjectSize;
                    return S_OK;
                });
        When(Method(valuesMock, IGetUnsignedLargeIntegerValue).Using(Ne(WPD_OBJECT_SIZE), _)).AlwaysReturn(~S_OK);


        When(Method(valuesMock, IGetValue).Using(Eq(WPD_OBJECT_DATE_CREATED), _)).AlwaysDo(
                [&expectedCreationDate](auto k, auto propVar) {
                    PropVariantInit(propVar);

                    double date;
                    SystemTimeToVariantTime(&expectedCreationDate, &date);
                    InitPropVariantFromDouble(date, propVar);

                    return S_OK;
                });
        When(Method(valuesMock, IGetValue).Using(Ne(WPD_OBJECT_DATE_CREATED), _)).AlwaysReturn(~S_OK);


        When(Method(valuesMock, IGetGuidValue).Using(Eq(WPD_FUNCTIONAL_OBJECT_CATEGORY), _)).AlwaysDo(
                [&expectedObjectCategory](auto k, auto g) {
                    *g = expectedObjectCategory;
                    return S_OK;
                });
        When(Method(valuesMock, IGetGuidValue).Using(Ne(WPD_FUNCTIONAL_OBJECT_CATEGORY), _)).AlwaysReturn(~S_OK);


        IPortableDevice &d = deviceMock.get();
        IPortableDeviceKeyCollection &dk = deviceKeyMock.get();
        IPortableDeviceValues &clientInfo = clientInfoMock.get();
        {
            WPDDevice device{devicePath, mockToPtr(d), mockToPtr(dk), mockToPtr(clientInfo)};
            WPD::WPDObject object{&device, objectPath};

            REQUIRE(object.getPath() == objectPath);

            // Check strings
            {
                REQUIRE(object.getFileSystem().empty());
                REQUIRE(object.getFileName() == expectedObjectName);

                std::wstring name;
                REQUIRE(device.getFileName(objectPath, &name));
                REQUIRE(name == expectedObjectName);

                REQUIRE_FALSE(device.getFileSystem(objectPath, &name));
            }

            // Check uint64
            {
                REQUIRE(object.getFileSize() == expectedObjectSize);
                REQUIRE(object.getSpace() == 0);
                REQUIRE(object.getSpaceFree() == 0);

                uint64_t size;
                REQUIRE(device.getFileSize(objectPath, &size));
                REQUIRE(size == expectedObjectSize);

                REQUIRE_FALSE(device.getSpace(objectPath, &size));
                REQUIRE_FALSE(device.getSpaceFree(objectPath, &size));
            }

            // Check dates
            {
                REQUIRE(object.getFileDateCreated().wYear == expectedCreationDate.wYear);
                REQUIRE(object.getFileDateModified().wYear == 0);

                SYSTEMTIME date;
                REQUIRE(device.getFileDateCreated(objectPath, &date));
                REQUIRE(date.wYear == expectedCreationDate.wYear);

                REQUIRE_FALSE(device.getFileDateModified(objectPath, &date));
            }

            // Check GUID
            {
                GUID objCategory = object.getFunctionalObjectCategory();
                REQUIRE(memcmp(&objCategory, &expectedObjectCategory, sizeof(GUID)) == 0);
                REQUIRE(object.getObjectType().Data1 == 0);

                GUID guid;
                REQUIRE(device.getFunctionalObjectCategory(objectPath, &guid));
                REQUIRE(memcmp(&guid, &expectedObjectCategory, sizeof(GUID)) == 0);

                REQUIRE_FALSE(device.getObjectType(objectPath, &guid));
            }

            {
                REQUIRE(object.isFile());
                REQUIRE_FALSE(object.isDirectory());
                REQUIRE_FALSE(object.isStorage());

                REQUIRE_FALSE(device.isFile(objectPath));
                REQUIRE_FALSE(device.isDirectory(objectPath));
                REQUIRE_FALSE(device.isStorage(objectPath));
            }
        }

        Verify(Method(deviceMock, IOpen).Using(EqSTR<LPCWSTR>(devicePath), _)).Once();

        Verify(Method(deviceMock, IRelease)).Once();
        Verify(Method(contentMock, IRelease)).Once();
        Verify(Method(deviceKeyMock, IRelease)).Once();
        Verify(Method(resourceMock, IRelease)).Once();
        Verify(Method(propertiesMock, IRelease)).Once();
        Verify(Method(valuesMock, IRelease)).AtLeastOnce();
        Verify(Method(clientInfoMock, IRelease)).Once();

    }

    SECTION("WPDObjectIterator test") {
        PortableDeviceWrapper deviceStub;
        PortableDeviceContentWrapper contentStub;
        PortableDeviceKeyCollectionWrapper keyCollectionStub;
        PortableDeviceResourcesWrapper resourceStub;
        PortableDevicePropertiesWrapper propertiesStub;
        PortableDeviceValuesWrapper clientInfoStub;
        PortableDeviceEnumWrapper enumStub;

        Mock<PortableDeviceWrapper> deviceMock{deviceStub};
        Mock<PortableDeviceContentWrapper> contentMock{contentStub};
        Mock<PortableDeviceKeyCollectionWrapper> deviceKeyMock{keyCollectionStub};
        Mock<PortableDeviceResourcesWrapper> resourceMock{resourceStub};
        Mock<PortableDevicePropertiesWrapper> propertiesMock{propertiesStub};
        Mock<PortableDeviceValuesWrapper> clientInfoMock{clientInfoStub};
        Mock<PortableDeviceEnumWrapper> enumMock{enumStub};

        const std::wstring devicePath = L"Some device ID";
        const std::wstring objectPath = L"Parent object path";

        const std::vector<std::wstring> firstPathes = {
                L"Object Path 1", L"Object Path 2", L"skipped"
        };

        const std::vector<std::wstring> secondPathes = {
                L"skipped", L"Object Path 5", L"Object Path 6"
        };

        mockConstructor(deviceMock, contentMock, resourceMock, propertiesMock, clientInfoMock, devicePath);

        Fake(Method(deviceKeyMock, IClear));
        Fake(Method(deviceKeyMock, IAdd));
        Fake(Method(deviceKeyMock, IRelease));
        Fake(Method(enumMock, IRelease));

        IEnumPortableDeviceObjectIDs &e = enumMock.get();
        When(Method(contentMock, IEnumObjects).Using(_, EqSTR<LPCWSTR>(objectPath), _, _)).AlwaysDo(
                [&e](auto, auto, auto, auto p) {
                    *p = &e;
                    return S_OK;
                });

        When(Method(enumMock, INext)).Do([&firstPathes](auto request, auto arr, auto size) {
            REQUIRE(request >= firstPathes.size());

            copyStrings(firstPathes, arr);
            *size = firstPathes.size();

            return S_OK;
        }).Do([&secondPathes](auto request, auto arr, auto size) {
            REQUIRE(request >= secondPathes.size());

            copyStrings(secondPathes, arr);
            *size = secondPathes.size();

            return S_OK;
        }).Do([](auto, auto, auto size) {
            *size = 0;
            return S_OK;
        });

        Fake(Method(enumMock, ISkip));

        IPortableDevice &d = deviceMock.get();
        IPortableDeviceKeyCollection &dk = deviceKeyMock.get();
        IPortableDeviceValues &clientInfo = clientInfoMock.get();
        {
            WPDDevice device{devicePath, mockToPtr(d), mockToPtr(dk), mockToPtr(clientInfo)};
            WPD::WPDObject object{&device, objectPath};

            {
                auto it = object.begin();
                auto end = object.end();

                REQUIRE(*it == firstPathes[0]);
                REQUIRE_FALSE(it == end);
                ++it;
                REQUIRE(*it == firstPathes[1]);
                REQUIRE_FALSE(it == end);

                // Skip firstPathes[2] and next element
                it.skip(2);
                Verify(Method(enumMock, ISkip).Using(Eq<ULONG>(1))).Once();

                REQUIRE(*it == secondPathes[0]);
                REQUIRE_FALSE(it == end);
                it.skip(1);
                REQUIRE(*it == secondPathes[2]);
                REQUIRE_FALSE(it == end);
                ++it;
                REQUIRE(it == end);
            }
        }

        Verify(Method(deviceMock, IOpen).Using(EqSTR<LPCWSTR>(devicePath), _)).Once();

        Verify(Method(deviceMock, IRelease)).Once();
        Verify(Method(contentMock, IRelease)).Once();
        Verify(Method(deviceKeyMock, IRelease)).Once();
        Verify(Method(resourceMock, IRelease)).Once();
        Verify(Method(propertiesMock, IRelease)).Once();
        Verify(Method(clientInfoMock, IRelease)).Once();
        Verify(Method(enumMock, IRelease)).AtLeastOnce();

    }

    SECTION("Empty WPDObjectIterator test") {
        PortableDeviceWrapper deviceStub;
        PortableDeviceContentWrapper contentStub;
        PortableDeviceKeyCollectionWrapper keyCollectionStub;
        PortableDeviceResourcesWrapper resourceStub;
        PortableDevicePropertiesWrapper propertiesStub;
        PortableDeviceValuesWrapper clientInfoStub;
        PortableDeviceEnumWrapper enumRootStub;

        Mock<PortableDeviceWrapper> deviceMock{deviceStub};
        Mock<PortableDeviceContentWrapper> contentMock{contentStub};
        Mock<PortableDeviceKeyCollectionWrapper> deviceKeyMock{keyCollectionStub};
        Mock<PortableDeviceResourcesWrapper> resourceMock{resourceStub};
        Mock<PortableDevicePropertiesWrapper> propertiesMock{propertiesStub};
        Mock<PortableDeviceValuesWrapper> clientInfoMock{clientInfoStub};
        Mock<PortableDeviceEnumWrapper> rootEnumMock{enumRootStub};

        const std::wstring devicePath = L"Some device ID";

        mockConstructor(deviceMock, contentMock, resourceMock, propertiesMock, clientInfoMock, devicePath);

        Fake(Method(deviceKeyMock, IClear));
        Fake(Method(deviceKeyMock, IAdd));
        Fake(Method(deviceKeyMock, IRelease));
        Fake(Method(rootEnumMock, IRelease));

        IEnumPortableDeviceObjectIDs &eRoot = rootEnumMock.get();
        When(Method(contentMock, IEnumObjects).Using(_, EqSTR<LPCWSTR>(WPD_DEVICE_OBJECT_ID), _, _)).Do(
                [&eRoot](auto, auto, auto, auto p) {
                    *p = &eRoot;
                    return S_OK;
                });

        When(Method(rootEnumMock, INext)).Do([](auto request, auto arr, auto size) {
            *size = 0;
            return S_OK;
        });

        IPortableDevice &d = deviceMock.get();
        IPortableDeviceKeyCollection &dk = deviceKeyMock.get();
        IPortableDeviceValues &clientInfo = clientInfoMock.get();
        {
            WPDDevice device{devicePath, mockToPtr(d), mockToPtr(dk), mockToPtr(clientInfo)};
            WPD::WPDObject root{&device, L""};

            {
                auto it = root.begin();
                REQUIRE(it == root.end());
                REQUIRE_FALSE(it != root.end());
                REQUIRE((*it).empty());
            }
        }

        Verify(Method(deviceMock, IOpen).Using(EqSTR<LPCWSTR>(devicePath), _)).Once();

        Verify(Method(deviceMock, IRelease)).Once();
        Verify(Method(contentMock, IRelease)).Once();
        Verify(Method(deviceKeyMock, IRelease)).Once();
        Verify(Method(resourceMock, IRelease)).Once();
        Verify(Method(propertiesMock, IRelease)).Once();
        Verify(Method(clientInfoMock, IRelease)).Once();
        Verify(Method(rootEnumMock, IRelease)).Once();

    }
}