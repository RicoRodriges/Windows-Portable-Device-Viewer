#include <Windows.h>
#include <iostream>
#include "WPDException.h"
#include "WPDEnumerator.h"

using namespace std;
using namespace WPD;

void enumerate_devices() {
    try {
        WPDEnumerator wpdEnumerator;

        int deviceNum = wpdEnumerator.getDeviceCount();
        cout << "Found " << deviceNum << " Windows portable device(s)" << endl;

        const vector<std::wstring> &pathes = wpdEnumerator.getDevicePathes();
        for (int i = 0; i < pathes.size(); ++i) {
            cout << endl << "Device #" << i << endl;

            const wstring &path = pathes[i];
            wcout << L"Path: " << path << endl;
            wcout << L"Name: " << wpdEnumerator.getFriendlyName(path) << endl;
            wcout << L"Manufacturer: " << wpdEnumerator.getManufacturer(path) << endl;
            wcout << L"Description: " << wpdEnumerator.getDescription(path) << endl;
        }
    }
    catch (WPDException &e) {
        cout << "Enumerator error: " << e.what() << endl;
        cout << "HRESULT: " << (void *) e.code() << " - " << e.explain() << endl;
    }
}

int main() {

    setlocale(0, "");

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    enumerate_devices();

    CoUninitialize();

    system("pause");
    return 0;
}