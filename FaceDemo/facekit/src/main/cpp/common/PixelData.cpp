//
// Created by wilbert on 2026/1/15.
//

#include "PixelData.hpp"
#include "Log.hpp"
#include <cstring>
#include <algorithm>

namespace face {
    
    // ============ 构造函数 ============

    PixelData::~PixelData() {
        LOGE("PixelData::%s this:%p", __FUNCTION__, this);
        clear();
    }

    PixelData::PixelData() : mFormat(PixelFormat::I420P), mPixels(nullptr), 
                            mPixelSize(0), mCapacity(0), mNeedFreePixel(false) {}
    
    PixelData::PixelData(PixelFormat format) 
        : mFormat(format), mPixels(nullptr), mPixelSize(0), mCapacity(0), mNeedFreePixel(false) {
        LOGE("PixelData:: format:%d", format);
    }
    
    PixelData::PixelData(PixelFormat format, size_t capacity) 
        : mFormat(format), mPixelSize(0), mNeedFreePixel(true) {
        LOGE("PixelData:: format:%d capacity:%zu", format, capacity);
        mPixels = static_cast<uint8_t*>(malloc(capacity));
        mCapacity = capacity;
    }
    
    // ============ 拷贝和移动语义 ============
    
    PixelData::PixelData(const PixelData& other) 
        : mFormat(other.mFormat), mPixelSize(other.mPixelSize), 
          mCapacity(other.mPixelSize), mNeedFreePixel(true), 
          mResolution(other.mResolution), mLabel(other.mLabel) {
        LOGE("PixelData:: operator=(const PixelData& other)");
        if (other.mPixelSize > 0 && other.mPixels) {
            mPixels = static_cast<uint8_t*>(malloc(other.mPixelSize));
            std::memcpy(mPixels, other.mPixels, other.mPixelSize);
        } else {
            mPixels = nullptr;
        }
    }
    
    PixelData& PixelData::operator=(const PixelData& other) {
        auto compare = this != &other;
        LOGE("PixelData:: operator=(const PixelData& other), compare:%d", compare);

        if (compare) {
            mFormat = other.mFormat;
            mResolution = other.mResolution;
            mLabel = other.mLabel;
            if (other.mPixelSize > 0 && other.mPixels) {
                if ((mNeedFreePixel && mCapacity < other.mPixelSize) || !mNeedFreePixel) {
                    clear();
                }
                if (mPixels == nullptr || mCapacity < other.mPixelSize) {
                    mCapacity = other.mPixelSize;
                    mPixels = static_cast<uint8_t*>(malloc(mCapacity));
                    mNeedFreePixel = true;
                }
                mPixelSize = other.mPixelSize;
                std::memcpy(mPixels, other.mPixels, mPixelSize);
            } else {
                mPixelSize = 0;
            }
        }
        return *this;
    }
    
    PixelData::PixelData(PixelData&& other) noexcept 
        : mFormat(other.mFormat), mPixels(other.mPixels), 
          mPixelSize(other.mPixelSize), mCapacity(other.mCapacity), 
          mNeedFreePixel(other.mNeedFreePixel), 
          mResolution(std::move(other.mResolution)), 
          mLabel(std::move(other.mLabel)) {
        LOGE("PixelData:: (PixelData&& other");
        other.mPixels = nullptr;
        other.mPixelSize = 0;
        other.mCapacity = 0;
        other.mNeedFreePixel = false;
    }
    
    PixelData& PixelData::operator=(PixelData&& other) noexcept {
        auto compare = this != &other;
        LOGE("PixelData:: operator=(PixelData&& other), compare:%d", compare);
        if (this != &other) {
            clear();
            mFormat = other.mFormat;
            mPixels = other.mPixels;
            mPixelSize = other.mPixelSize;
            mCapacity = other.mCapacity;
            mNeedFreePixel = other.mNeedFreePixel;
            mResolution = std::move(other.mResolution);
            mLabel = std::move(other.mLabel);
            
            other.mPixels = nullptr;
            other.mPixelSize = 0;
            other.mCapacity = 0;
            other.mNeedFreePixel = false;
        }
        return *this;
    }
    
    // ============ 像素数据分配 ============
    
    void PixelData::allocate(const uint8_t* data, size_t size) {
        if (data && size > 0) {
            clear();
            mPixels = static_cast<uint8_t*>(malloc(size));
            std::memcpy(mPixels, data, size);
            mPixelSize = size;
            mCapacity = size;
            mNeedFreePixel = true;
        }
    }
    
    void PixelData::allocate(size_t size) {
        if (size > 0) {
            clear();
            mPixels = static_cast<uint8_t*>(malloc(size));
            mCapacity = size;
            mPixelSize = 0;
            mNeedFreePixel = true;
        }
    }
    
    // ============ 数据访问 ============
    
    void PixelData::copyPixels(uint8_t* dest, size_t destSize) const {
        if (dest && destSize > 0 && mPixels && mPixelSize > 0) {
            size_t copySize = std::min(destSize, (size_t)mPixelSize);
            std::memcpy(dest, mPixels, copySize);
        }
    }
    
    void PixelData::copyFrom(const PixelData& other) {
        *this = other;
    }
    
    void PixelData::append(const uint8_t* data, size_t size) {
        if (data && size > 0) {
            if (mPixelSize + size > mCapacity) {
                // 需要扩容
                uint32_t newCapacity = mPixelSize + size;
                auto newPixels = static_cast<uint8_t*>(malloc(newCapacity));
                if (mPixels && mPixelSize > 0) {
                    std::memcpy(newPixels, mPixels, mPixelSize);
                    if (mNeedFreePixel) {
                        free(mPixels);
                    }
                }
                mPixels = newPixels;
                mCapacity = newCapacity;
                mNeedFreePixel = true;
            }
            std::memcpy(mPixels + mPixelSize, data, size);
            mPixelSize += size;
        }
    }
    
    void PixelData::setPixelData(uint8_t *pixel,
                                 uint32_t width,
                                 uint32_t height,
                                 face::PixelFormat format,
                                 uint32_t size,
                                 bool copy) {
        uint32_t pixelSize = width * height * (uint32_t)getBytesPerPixel(format);
        if (size == 0) {
            size = pixelSize;
        }
        LOGE("PixelData::setPixelData,pixel:%p, %dx%d, format:%d, size:%d, pixelSize:%d", pixel, width, height, format, size, pixelSize);
        if (copy && size > mCapacity) {
            reserve(size);
            std::memcpy(mPixels, pixel, size);
        } else {
            mPixels = pixel;
        }
        mResolution.setSize(width, height);
        mFormat = format;
        mCapacity = size;
        mPixelSize = pixelSize;
        mNeedFreePixel = false;  // 外部传入的指针，不负责释放
    }
    
    // ============ 运算符重载 ============
    
    bool PixelData::operator==(const PixelData& other) const {
        // 首先比较格式、分辨率和大小
        if (mFormat != other.mFormat) {
            return false;
        }
        
        if (mResolution != other.mResolution) {
            return false;
        }
        
        if (mPixelSize != other.mPixelSize) {
            return false;
        }
        
        // 比较像素数据
        if (mPixelSize == 0) {
            return true;  // 都是空的，视为相等
        }
        
        if (!mPixels || !other.mPixels) {
            return false;  // 其中一个为空，不相等
        }
        
        return mPixels == other.mPixels;
    }

    uint32_t PixelData::calculateDataSize() const  {
        int width = mResolution.getWidth();
        int height = mResolution.getHeight();
        if (width == 0 || height == 0) {
            return 0;
        }
        return (uint32_t)getBytesPerPixel(mFormat) * width * height;
    }

    bool PixelData::validateDataSize() const {
        return getPixelSize() == calculateDataSize();
    }

    void PixelData::setPixelByte(size_t index, uint8_t value) {
        if (index < mPixelSize) {
            mPixels[index] = value;
        }
    }

    uint8_t PixelData::getPixelByte(size_t index) const {
        if (index < mPixelSize) {
            return mPixels[index];
        }
        return 0;
    }

    void PixelData::reserve(size_t capacity) {
        clear();
        mPixels = static_cast<uint8_t *>(malloc(capacity));
        mNeedFreePixel = true;
        mCapacity = capacity;
        mPixelSize = 0;
    }

    void PixelData::clear() {
        if (mNeedFreePixel && mPixels) {
            free(mPixels);
        }
        mPixels = nullptr;
        mNeedFreePixel = false;
        mCapacity = 0;
        mPixelSize = 0;
    }

    float PixelData::getBytesPerPixel(face::PixelFormat format) {
        switch (format) {
            case PixelFormat::RGB:
            case PixelFormat::BGR:
                return 3;
            case PixelFormat::ARGB:
            case PixelFormat::RGBA:
                return 4;
            case PixelFormat::I420P:
            case PixelFormat::NV21:
                return 1.5f; // 平面格式，这里返回1
            default:
                return 0;
        }
    }

    const char *PixelData::getFormatString() const {
        switch (mFormat) {
            case PixelFormat::I420P:
                return "I420P";
            case PixelFormat::NV21:
                return "NV21";
            case PixelFormat::ARGB:
                return "ARGB";
            case PixelFormat::RGBA:
                return "RGBA";
            case PixelFormat::RGB:
                return "RGB";
            case PixelFormat::BGR:
                return "BGR";
            default:
                return "UNKNOWN";
        }
    }
} // face