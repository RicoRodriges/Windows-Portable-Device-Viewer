#include "WPDUtils.h"
#include "WPDObjectIterator.h"
#include "WPDDevice.h"

#include "WPDObject.h"

#include <utility>

WPD::WPDObject::WPDObject(const WPDDevice *d, const std::wstring &p) : device(d), path(p), init(false) {}

WPD::WPDObject::WPDObject(const WPDDevice *d, std::wstring &&p) : device(d), path(p), init(false) {}

std::wstring WPD::WPDObject::getPath() const {
    return path;
}

void WPD::WPDObject::firstInit() const {
    if (!this->init) {
        this->updateFields();
    }
}

std::wstring WPD::WPDObject::getFileName() const {
    firstInit();
    return name;
}

GUID WPD::WPDObject::getObjectType() const {
    firstInit();
    return objectType;
}

GUID WPD::WPDObject::getFunctionalObjectCategory() const {
    firstInit();
    return functionalObjectCategory;
}

uint64_t WPD::WPDObject::getSpaceFree() const {
    firstInit();
    return spaceFree;
}

uint64_t WPD::WPDObject::getSpace() const {
    firstInit();
    return space;
}

std::wstring WPD::WPDObject::getFileSystem() const {
    firstInit();
    return fileSystem;
}

uint64_t WPD::WPDObject::getFileSize() const {
    firstInit();
    return fileSize;
}

SYSTEMTIME WPD::WPDObject::getFileDateModified() const {
    firstInit();
    return fileDateModified;
}

SYSTEMTIME WPD::WPDObject::getFileDateCreated() const {
    firstInit();
    return fileDateCreated;
}

bool WPD::WPDObject::isFile() const {
    firstInit();
    return WPD::Utils::isFile(objectType);
}

bool WPD::WPDObject::isDirectory() const {
    firstInit();
    return WPD::Utils::isDirectory(objectType);
}

bool WPD::WPDObject::isStorage() const {
    firstInit();
    return WPD::Utils::isStorage(objectType, functionalObjectCategory);
}

void WPD::WPDObject::updateFields() const {
    device->initObject(this);
    init = true;
}

WPD::WPDObjectIterator WPD::WPDObject::begin() const {
    return device->getFileIterator(path);
}

WPD::WPDObjectIterator WPD::WPDObject::end() const {
    return WPDObjectIterator::endIterator(device, path);
}

