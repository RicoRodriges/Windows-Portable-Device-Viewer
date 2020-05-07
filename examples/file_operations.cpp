#include <Windows.h>
#include <iostream>
#include "WPDException.h"
#include "WPDObject.h"
#include "WPDDevice.h"
#include "WPDObjectIterator.h"

using namespace std;
using namespace WPD;

static const wstring someDevicePath = L"";
static const wstring someFilePath = L"o91909EF2";

void file_operations() {
    try {
        WPDDevice device(someDevicePath);
        WPDObject file = device.getObject(someFilePath);

        // Copy file to PC without dates
        // device.downloadFile(L"o91909EF2", L"C:\\MyFolder\\123.txt");

        // or copy file to PC with dates
        SYSTEMTIME created = file.getFileDateCreated();
        SYSTEMTIME modified = file.getFileDateModified();
        device.downloadFile(file.getPath(), L"C:\\MyFolder\\" + file.getFileName(), created, modified);

        // Or read file to buffer (it is not an ASCII string)
        string buffer = device.readFile(file.getPath());

        // Or do something with file
        int part = 0;
        device.readFile(file.getPath(), 1024, [&part](const char *buf, uint32_t size) {
            string filePart{buf, size};
            cout << "File part #" << part << endl;
            cout << filePart.c_str() << endl;
            part++;
        });

    }
    catch (WPDException &e) {
        cout << "Error: " << e.what() << endl;
        cout << "HRESULT: " << (void *) e.code() << " - " << e.explain() << endl;
    }
}

int main() {

    setlocale(0, "");

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    file_operations();

    CoUninitialize();

    system("pause");
    return 0;
}