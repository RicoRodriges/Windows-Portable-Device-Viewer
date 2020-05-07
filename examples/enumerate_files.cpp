#include <Windows.h>
#include <iostream>
#include "WPDException.h"
#include "WPDObject.h"
#include "WPDEnumerator.h"
#include "WPDDevice.h"
#include "WPDObjectIterator.h"

using namespace std;
using namespace WPD;

void printDate(const SYSTEMTIME &date) {
    char dateStr[100];
    GetDateFormatA(LOCALE_USER_DEFAULT, 0, &date, nullptr, dateStr, 100);
    cout << dateStr << " ";
    GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &date, nullptr, dateStr, 100);
    cout << dateStr;
}

void printSize(const ULONGLONG size) {
    if (size < 1024)
        cout << size << " b";
    else if (size < 1024 * 1024)
        cout << (float) size / 1024 << " Kb";
    else if (size < 1024 * 1024 * 1024)
        cout << (float) size / 1024 / 1024 << " Mb";
    else
        cout << (float) size / 1024 / 1024 / 1024 << " Gb";
}

void enumerate_files() {
    try {
        WPDEnumerator wpdEnumerator;

        auto pathes = wpdEnumerator.getDevicePathes();
        if (pathes.empty())
            return;

        const wstring &path = pathes[0];

        WPDDevice device(path);

        // Get root directory
        WPDObject rootDirectory = device.getObject(L"");
        // or some directory
        //path = L"o91900000";
        //WPDObject item = device.getObject(path);
        cout << "Device #0 root directory:" << endl;

        for (const wstring &itemPath : rootDirectory) {
            wcout << endl << L"Path: " << itemPath << endl;

            auto item = device.getObject(itemPath);

            wcout << L"Name: " << item.getFileName() << endl;

            if (item.isStorage()) {
                cout << "Storage" << endl;

                uint64_t total = item.getSpace();
                cout << "Total space: ";
                printSize(total);
                cout << endl;

                uint64_t free = item.getSpaceFree();
                cout << "Free space: ";
                printSize(free);
                cout << endl;

                wstring FS = item.getFileSystem();
                wcout << L"File System: " << FS << endl;
            } else {
                if (item.isDirectory())
                    cout << "Directory" << endl;
                else if (item.isFile())
                    cout << "File" << endl;
                else
                    cout << "Unsupported item type" << endl;

                uint64_t size = item.getFileSize();
                cout << "File size: ";
                printSize(size);
                cout << endl;

                SYSTEMTIME created = item.getFileDateCreated();
                cout << "Created: ";
                printDate(created);
                cout << endl;

                SYSTEMTIME modified = item.getFileDateModified();
                cout << "Modified: ";
                printDate(modified);
                cout << endl;
            }
            cout << endl;
        }
    }
    catch (WPDException &e) {
        cout << "Error: " << e.what() << endl;
        cout << "HRESULT: " << (void *) e.code() << " - " << e.explain() << endl;
    }
}

int main() {

    setlocale(0, "");

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    enumerate_files();

    CoUninitialize();

    system("pause");
    return 0;
}