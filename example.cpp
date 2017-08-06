#include <Windows.h>
#include <iostream>
#include <comdef.h>
#include "WPDEnumerator.h"
#include "WPDevice.h"

using namespace std;

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
		cout << (float)size / 1024 << " Kb";
	else if (size < 1024 * 1024 * 1024)
		cout << (float)size / 1024 / 1024 << " Mb";
	else
		cout << (float)size / 1024 / 1024 / 1024 << " Gb";
}

void example1() {
	try {
		WPDEnumerator Enum;

		int deviceNum = Enum.getDeviceCount();
		cout << "Found " << deviceNum << " Windows portable device(s)" << endl;

		wstring path;
		for (int i = 0; i < deviceNum; i++) {
			cout << endl << "Device #" << i << endl;

			if (Enum.getDevicePath(i, &path)) {
				wcout << L"Path: " << path << endl;
				wstring str;
				if (Enum.getFriendlyName(path, &str))
					wcout << L"Name: " << str << endl;
				if (Enum.getManufacturer(path, &str))
					wcout << L"Manufacturer: " << str << endl;
				if (Enum.getDescription(path, &str))
					wcout << L"Description: " << str << endl;
			}
			else {
				cout << "Error getDevicePath. error = " << (void*)Enum.getError() << endl;
			}
		}
	}
	catch (WPDException& e) {
		_com_error err(e.code());
		cout << "Enumerator error: " << e.what() << endl;
		cout << "HRESULT: " << (void*)e.code() << " - " << err.ErrorMessage();
	}
}

void example2() {
	try {
		WPDEnumerator Enum;

		int deviceNum = Enum.getDeviceCount();
		if (deviceNum == 0)
			return;

		wstring path;
		Enum.getDevicePath(0, &path);

		try {
			WPDevice device(path);
			WPDFileEnumerator fileEnum;

			// Get root directory
			device.getFileEnumerator(L"", fileEnum);
			// or some directory
			//path = L"o91900000";
			//device.getFileEnumerator(path, fileEnum);
			cout << "Device #0 root directory:" << endl;

			wstring files[5];
			int received;
			while (received = fileEnum.getNextFile(files, 5)) {
				for (int i = 0; i < received; i++) {
					wcout << endl << L"Path: " << files[i].c_str() << endl;

					wstring name;
					if (device.getFileName(files[i], &name))
						wcout << L"Name: " << name.c_str() << endl;

					GUID guid;
					device.getObjectType(files[i], &guid);
					if (IsEqualGUID(guid, WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT)) {
						device.getFunctionalObjectCategoty(files[i], &guid);
						if (IsEqualGUID(guid, WPD_FUNCTIONAL_CATEGORY_STORAGE)) {
							cout << "Storage" << endl;

							ULONGLONG total;
							if (device.getSpace(files[i], &total)) {
								cout << "Total space: ";
								printSize(total);
								cout << endl;
							}

							ULONGLONG free;
							if (device.getSpaceFree(files[i], &free)) {
								cout << "Free space: ";
								printSize(free);
								cout << endl;
							}

							wstring FS;
							if (device.getFileSystem(files[i], &FS))
								wcout << L"File System: " << FS << endl;

						}
						else
							cout << "Unsupported functional object" << endl;
					}
					else {
						if (IsEqualGUID(guid, WPD_CONTENT_TYPE_FOLDER))
							cout << "Directory" << endl;
						else
							cout << "File" << endl;

						ULONGLONG size;
						if (device.getFileSize(files[i], &size)) {
							cout << "File size: "; printSize(size); cout << endl;
						}

						SYSTEMTIME date;
						if (device.getFileDateCreated(files[i], &date)) {
							cout << "Created: ";
							printDate(date);
							cout << endl;
						}
						if (device.getFileDateModified(files[i], &date)) {
							cout << "Modified: ";
							printDate(date);
							cout << endl;
						}
					}
					cout << endl;
				}
			}
		}
		catch (WPDException& e) {
			_com_error err(e.code());
			cout << "Device error: " << e.what() << endl;
			cout << "HRESULT: " << (void*)e.code() << " - " << err.ErrorMessage();
		}
	}
	catch (WPDException& e) {
		_com_error err(e.code());
		cout << "Enumerator error: " << e.what() << endl;
		cout << "HRESULT: " << (void*)e.code() << " - " << err.ErrorMessage();
	}
}

void example3() {
	try {
		WPDEnumerator Enum;

		int deviceNum = Enum.getDeviceCount();
		if (deviceNum == 0)
			return;

		wstring path;
		Enum.getDevicePath(0, &path);

		try {
			WPDevice device(path);

			// Copy a file to the PC without saving a dates
			// device.copyFile(L"o91909EF2", L"C:\\MyFolder\\123.txt");

			SYSTEMTIME create, modif;
			FILETIME fcreate, fmodif, temp;
			FILETIME *createTime = nullptr, *modifTime = nullptr;

			if (device.getFileDateCreated(L"o91909EF2", &create)) {
				createTime = &fcreate;
				SystemTimeToFileTime(&create, &temp);
				LocalFileTimeToFileTime(&temp, &fcreate);
			}
			if (device.getFileDateModified(L"o91909EF2", &modif)) {
				modifTime = &fmodif;
				SystemTimeToFileTime(&modif, &temp);
				LocalFileTimeToFileTime(&temp, &fmodif);
			}

			device.copyFile(L"o91909EF2", L"C:\\MyFolder\\123.txt", createTime, modifTime);
		}
		catch (WPDException& e) {
			_com_error err(e.code());
			cout << "Device error: " << e.what() << endl;
			cout << "HRESULT: " << (void*)e.code() << " - " << err.ErrorMessage();
		}
	}
	catch (WPDException& e) {
		_com_error err(e.code());
		cout << "Enumerator error: " << e.what() << endl;
		cout << "HRESULT: " << (void*)e.code() << " - " << err.ErrorMessage();
	}
}

int main() {

	setlocale(0, "");

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	example1();

	CoUninitialize();

	system("pause");
	return 0;
}