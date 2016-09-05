# Windows portable device viewer
This set of classes allows viewing files on portable devices such as IPhones, digital cameras and other windows portable devices (WPD). Currently implemented in read-only mode.
* `WPDEnumerator` allows to get information about all connected portable devices.
* `WPDevice` allows interaction with a particular device and its file system.
* `WPDFileEnumerator` displays all files in a specific directory.

## WPDEnumerator class
The `refreshDeviceList` method refreshes the list of devices that are connected to the computer.

The `getDeviceCount` method returns number of connected devices.

The `getDevicePath` method returns string to access the device with `deviceIndex` index. The `deviceIndex` can take values from `0` to `getDeviceCount()-1`.
``` cpp
bool getDevicePath (int deviceIndex, std :: wstring * path)
```
For information about the device, you can use one of the following methods:
``` cpp
bool getFriendlyName (const std :: wstring & path, std :: wstring * str);
bool getManufacturer (const std :: wstring & path, std :: wstring * str);
bool getDescription (const std :: wstring & path, std :: wstring * str);
```

## WPDevice class
Constructor opens the device. If `WPDevice::getError` returned 0, then device was successfully opened.
``` cpp
WPDevice (const std :: wstring & path);
```
The `getFileEnumerator` method returns enumerator, which contains all files in directory. If `path` is NULL, then enumerator contains all files (storages) from root folder. The `enumerator` mustn't be NULL.
``` cpp
bool getFileEnumerator (const std :: wstring * path, WPDFileEnumerator * enumerator);
```
The `getFileName` method returns name of the object/file.

The `getObjectType` method returns the GUID of the object (See more [MSDN](https://msdn.microsoft.com/en-us/library/windows/hardware/ff597558(v=vs.85).aspx)). If `guid` equal `WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT`, then you can call `getFunctionalObjectCategoty` method. If it returns `WPD_FUNCTIONAL_CATEGORY_STORAGE`, then the file is storage.

For storages:
``` cpp
bool getSpaceFree (const std :: wstring & path, unsigned long long * size);
bool getSpace (const std :: wstring & path, unsigned long long * size);
bool getFileSystem (const std :: wstring & path, std :: wstring * str);
```
For other files:
``` cpp
bool getFileSize (const std :: wstring & path, unsigned long long * size);
bool getFileDateModified (const std :: wstring & path, SYSTEMTIME * dateModif);
bool getFileDateCreated (const std :: wstring & path, SYSTEMTIME * dateCreate);
bool copyFile (const std :: wstring & path, const std :: wstring & newFile, const FILETIME * dateCreate = 0, const FILETIME * dateModif = 0);
```

## WPDFileEnumerator class
For enumeration of files in directory, use the function `WPDevice::getFileEnumerator`.

The `getNextFile` method returns the number of files stored in array `paths` size `pathsNum`.
``` cpp
int getNextFile (std :: wstring * paths, int pathsNum);
```
The `Skip` skips a specified number of objects. The `Reset` resets the enumerator to the beginning.
