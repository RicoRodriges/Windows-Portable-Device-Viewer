#include "shared_tests.h"

#include "WPDEnumerator.h"

using namespace fakeit;

class WPDEnumerator : public WPD::WPDEnumerator {
public:
    WPDEnumerator(ptr<IPortableDeviceManager> &&dm) :
            WPD::WPDEnumerator(std::move(dm)) {}
};

void mockConstructorCalls(Mock<PortableDeviceManagerWrapper> &mock) {
    When(Method(mock, IRefreshDeviceList)).Return(S_FALSE);
    When(Method(mock, IGetDevices)).Return(S_FALSE);
    Fake(Method(mock, IRelease));
}

TEST_CASE("WPDEnumerator class tests") {

    SECTION("success without devices") {
        PortableDeviceManagerWrapper stub;
        Mock<PortableDeviceManagerWrapper> mock{stub};

        mockConstructorCalls(mock);

        IPortableDeviceManager &d = mock.get();
        {
            WPDEnumerator enumerator{mockToPtr(d)};

            REQUIRE(enumerator.getDeviceCount() == 0);
            REQUIRE(enumerator.getDevicePathes().empty());
        }

        Verify(Method(mock, IGetDevices)).Once();
        Verify(Method(mock, IRelease)).Once();
    }

    SECTION("success with devices") {
        const std::vector<std::wstring> expectedPathes = {L"path 1", L"path 2", L"path 3"};
        const int expectedCount = expectedPathes.size();

        PortableDeviceManagerWrapper stub;
        Mock<PortableDeviceManagerWrapper> mock{stub};

        When(Method(mock, IRefreshDeviceList)).Return(S_OK);

        When(Method(mock, IGetDevices).Using(Eq<LPWSTR *>(nullptr), EqPTR<DWORD *>(0))).Do(
                [&expectedCount](LPWSTR *, DWORD *count) {
                    *count = expectedCount;
                    return S_OK;
                });
        When(Method(mock, IGetDevices).Using(Ne<LPWSTR *>(nullptr), EqPTR<DWORD *>(expectedCount))).Do(
                [&expectedPathes, &expectedCount](LPWSTR *pathes, DWORD *count) {
                    copyStrings(expectedPathes, pathes);
                    return S_OK;
                });
        Fake(Method(mock, IRelease));

        IPortableDeviceManager &d = mock.get();
        {
            WPDEnumerator enumerator{mockToPtr(d)};

            REQUIRE(enumerator.getDeviceCount() == expectedCount);
            REQUIRE(enumerator.getDevicePathes() == expectedPathes);
        }

        Verify(Method(mock, IGetDevices)).Twice();
        Verify(Method(mock, IRelease)).Once();
    }

    SECTION("test getFriendlyName success") {
        const std::wstring path = L"Some path";
        const std::wstring expectedName = L"Some name";

        PortableDeviceManagerWrapper stub;
        Mock<PortableDeviceManagerWrapper> mock{stub};

        mockConstructorCalls(mock);

        When(Method(mock, IGetDeviceFriendlyName).Using(EqSTR<LPCWSTR>(path), Eq<WCHAR *>(nullptr),
                                                        EqPTR<DWORD *>(0))).Do(
                [&expectedName](LPCWSTR requestedPath, WCHAR *buf, DWORD *size) {
                    *size = expectedName.size();
                    return S_OK;
                });
        When(Method(mock, IGetDeviceFriendlyName).Using(EqSTR<LPCWSTR>(path), Ne<WCHAR *>(nullptr),
                                                        EqPTR<DWORD *>(expectedName.size()))).Do(
                [&expectedName](LPCWSTR requestedPath, WCHAR *buf, DWORD *bufSize) {
                    const int size = *bufSize;
                    memcpy_s(buf, size * sizeof(wchar_t), expectedName.c_str(), expectedName.size() * sizeof(wchar_t));
                    return S_OK;
                });

        IPortableDeviceManager &d = mock.get();
        {
            WPDEnumerator enumerator{mockToPtr(d)};

            REQUIRE(enumerator.getFriendlyName(path) == expectedName);
        }
    }

    SECTION("test getManufacturer fail") {
        const std::wstring path = L"Some path";

        PortableDeviceManagerWrapper stub;
        Mock<PortableDeviceManagerWrapper> mock{stub};

        mockConstructorCalls(mock);

        When(Method(mock, IGetDeviceManufacturer).Using(EqSTR<LPCWSTR>(path), Eq<WCHAR *>(nullptr), EqPTR<DWORD *>(0)))
                .Return(S_FALSE);

        IPortableDeviceManager &d = mock.get();
        {
            WPDEnumerator enumerator{mockToPtr(d)};

            REQUIRE(enumerator.getManufacturer(path).empty());
        }

    }

    SECTION("test getDescription not found") {
        const std::wstring path = L"Some path";

        PortableDeviceManagerWrapper stub;
        Mock<PortableDeviceManagerWrapper> mock{stub};

        mockConstructorCalls(mock);

        When(Method(mock, IGetDeviceDescription).Using(EqSTR<LPCWSTR>(path), Eq<WCHAR *>(nullptr), EqPTR<DWORD *>(0)))
                .Return(HRESULT_FROM_WIN32(ERROR_NOT_FOUND));

        IPortableDeviceManager &d = mock.get();
        {
            WPDEnumerator enumerator{mockToPtr(d)};

            REQUIRE_THROWS_MATCHES(enumerator.getDescription(path), WPD::WPDException,
                                   Catch::Matchers::Predicate<WPD::WPDException>([](const WPD::WPDException &e) {
                                       return e.what() != nullptr && strlen(e.what()) > 0 &&
                                              e.code() != 0 && !e.explain().empty();
                                   }));
        }

    }
}