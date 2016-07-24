#include <Windows.h>
#include <iostream>
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
	else if (size < 1024*1024)
		cout << (float)size/1024 << " Kb";
	else if (size < 1024*1024*1024)
		cout << (float)size/1024/1024 << " Mb";
	else
		cout << (float)size/1024/1024/1024 << " Gb";
}

void example() {
	// Get all devices
	WPDEnumerator Enum;
	if (Enum.getError()) {
		HRESULT hr;
		int error = Enum.getError(&hr);
		cout << "Error create WPDEnumerator. error = " << error << ", hr = " << (void*)hr << endl;
		return;
	}

	int deviceNum = Enum.getDeviceCount();
	cout << "Found " << deviceNum << " Windows portable device(s)" << endl;

	wstring path;
	for (int i = 0; i < deviceNum; i++) {
		cout << endl << "Device #" << i << endl;

		if (Enum.getDevicePath(i, &path)) {
			wcout << L"Path: " << path.c_str() << endl;
			wstring str;
			if (Enum.getFriendlyName(path, &str))
				wcout << L"Name: " << str.c_str() << endl;
			if (Enum.getManufacturer(path, &str))
				wcout << L"Manufacturer: " << str.c_str() << endl;
			if (Enum.getDescription(path, &str))
				wcout << L"Description: " << str.c_str() << endl;
		} else {
			HRESULT hr;
			int error = Enum.getError(&hr);
			cout << "Error getDevicePath. error = " << error << ", hr = " << (void*)hr << endl;
		}
	}

	if (deviceNum != 0) {
		// Open first device
		Enum.getDevicePath(0, &path);
		WPDevice device(path);
		if (device.getError()) {
			HRESULT hr;
			int error = device.getError(&hr);
			cout << "Error create WPDevice. error = " << error << ", hr = " << (void*)hr << endl;
			return;
		}

		WPDFileEnumerator fileEnum;

		// Get root directory
		device.getFileEnumerator(nullptr, &fileEnum);
		// or some directory
		// path = L"s20001";
		// device.getFileEnumerator(&path, &fileEnum); 
		cout << endl << endl << "Device #0 root directory:" << endl;

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
						cout << "Drive" << endl;

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

					} else
						cout << "Unsupported functional object" << endl;
				} else {
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
						char dateStr[100];
						cout << "Created: ";
						printDate(date);
						cout << endl;
					}
					if (device.getFileDateModified(files[i], &date)) {
						char dateStr[100];
						cout << "Modified: ";
						printDate(date);
						cout << endl;
					}
				}

			}
		}

		/* Copy file to PC
		
		// device.copyFile(L"o91909EF2", L"C:\\MyFolder\\123.txt");

		SYSTEMTIME create, modif;
		FILETIME *createTime = nullptr, *modifTime = nullptr, temp;

		if (device.getFileDateCreated(L"o91909EF2", &create)) {
			createTime = new FILETIME;
			SystemTimeToFileTime(&create, &temp);
			LocalFileTimeToFileTime(&temp, createTime);
		}
		if (device.getFileDateModified(L"o91909EF2", &modif)) {
			modifTime = new FILETIME;
			SystemTimeToFileTime(&modif, &temp);
			LocalFileTimeToFileTime(&temp, modifTime);
		}

		device.copyFile(L"o91909EF2", L"C:\\MyFolder\\123.txt", createTime, modifTime);
		if (createTime)
			delete createTime;
		if (modifTime)
			delete modifTime; */

	}
}

int main() {

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	example();

	CoUninitialize();

	system("pause");
	return 0;
}