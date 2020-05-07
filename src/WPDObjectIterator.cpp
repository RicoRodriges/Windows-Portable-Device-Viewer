#include <cassert>

#include "WPDException.h"
#include "WPDObjectIterator.h"

WPD::WPDObjectIterator::WPDObjectIterator(EnumPortableDeviceObjectIDs &&p, const WPDDevice *device,
                                          std::wstring path) : iter(std::move(p)),
                                                               device(device),
                                                               path(std::move(path)),
                                                               offset(0),
                                                               pos(0),
                                                               ended(false) {
    nextFiles();
}

WPD::WPDObjectIterator WPD::WPDObjectIterator::endIterator(const WPD::WPDDevice *device, const std::wstring &path) {
    return WPD::WPDObjectIterator(EnumPortableDeviceObjectIDs {nullptr, [](IEnumPortableDeviceObjectIDs *) {}}, device, path);
}

bool WPD::WPDObjectIterator::operator==(const WPDObjectIterator &it) const {
    if (this->device != it.device || this->path != it.path) {
        return false;
    }
    return (this->ended && it.ended) || (!this->ended && !it.ended && this->pos == it.pos);
}

bool WPD::WPDObjectIterator::operator!=(const WPDObjectIterator &it) const {
    return !(*this == it);
}

std::wstring WPD::WPDObjectIterator::operator*() const {
    if (ended) {
        return L"";
    }
    return pathes[offset];
}

void WPD::WPDObjectIterator::operator++() {
    if (ended) return;

    ++pos;
    ++offset;
    if (offset >= pathes.size()) {
        nextFiles();
    }
}

void WPD::WPDObjectIterator::nextFiles() {
    if (!iter) {
        ended = true;
        return;
    }

    ULONG result = 0;
    auto deleter = [&result](wchar_t **p) {
        for (ULONG i = 0; i < result; i++) {
            CoTaskMemFree(p[i]);
        }
        delete[] p;
    };
    std::unique_ptr<wchar_t *[], decltype(deleter)> temp{new wchar_t *[batch], deleter};

    HRESULT hr = iter->Next(batch, temp.get(), &result);
    if (FAILED(hr)) throw WPDException(hr, "Couldn't get the next object");

    offset = 0;
    pathes.clear();

    if (result > 0) {
        pathes.reserve(batch);
        for (ULONG i = 0; i < result; i++) {
            pathes.emplace_back(temp[i]);
        }
    } else {
        ended = true;
    }
}

void WPD::WPDObjectIterator::skip(int num) {
    if (ended || !iter || num <= 0) return;

    int skip = num;

    const unsigned int cachedItems = pathes.size() - offset - 1;
    if (cachedItems > skip) {
        pos += num;
        offset += num;
        ++(*this);
    } else {
        skip -= cachedItems;
        assert(skip >= 0);
        if (skip == 0 || SUCCEEDED(iter->Skip(skip))) {
            pos += num;
            nextFiles();
        }
    }
}

void WPD::WPDObjectIterator::reset() {
    if (!iter) return;

    HRESULT hr = iter->Reset();
    if (SUCCEEDED(hr)) {
        ended = false;
        pos = 0;
        nextFiles();
    }
}
