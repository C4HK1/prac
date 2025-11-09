#pragma once

#include <stddef.h>
#include <utility>

template <typename T>
class SharedPtr {
private:
    T* rawPtr;
    size_t* refCount;

    void release() {
        if (refCount && --(*refCount) == 0) {
            delete rawPtr;
            delete refCount;
        }
    }

public:
    SharedPtr()
        : rawPtr(nullptr), refCount(nullptr) {}
    explicit SharedPtr(T* p)
        : rawPtr(p), refCount(new size_t(1)) {}

    SharedPtr(const SharedPtr& other)
        : rawPtr(other.rawPtr), refCount(other.refCount) {
        if (refCount)
            ++(*refCount);
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other)
        : rawPtr(static_cast<T*>(other.get())), refCount(other.use_count() ? other.refCount : nullptr) {
        if (refCount)
            ++(*refCount);
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release();
            rawPtr = other.rawPtr;
            refCount = other.refCount;
            if (refCount)
                ++(*refCount);
        }
        return *this;
    }

    ~SharedPtr() {
        release();
    }

    T& operator*() const {
        return *rawPtr;
    }
    T* operator->() {
        return rawPtr;
    }
    const T* operator->() const {
        return rawPtr;
    }

    size_t use_count() const {
        return refCount ? *refCount : 0;
    }

    void reset(T* p = nullptr) {
        release();
        rawPtr = p;
        refCount = p ? new size_t(1) : nullptr;
    }

    void swap(SharedPtr& other) {
        std::swap(rawPtr, other.rawPtr);
        std::swap(refCount, other.refCount);
    }

    T* get() const {
        return rawPtr;
    }

    bool operator==(const SharedPtr& other) const {
        return rawPtr == other.rawPtr;
    }
    bool operator!=(const SharedPtr& other) const {
        return rawPtr != other.rawPtr;
    }

    template <typename U>
    friend class SharedPtr;
};
