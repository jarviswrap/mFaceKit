//
// Created by wilbert on 2026/1/15.
//

#include "PixelData.hpp"
#include <cstring>
#include <algorithm>

namespace face
{
    
    PixelData::PixelData() : mFormat(PixelFormat::RGBA) {}
    
    PixelData::PixelData(PixelFormat format) : mFormat(format) {}
    
    PixelData::PixelData(PixelFormat format, size_t capacity) 
        : mFormat(format) {
        mPixels.reserve(capacity);
    }
    
    PixelData::PixelData(const PixelData& other) 
        : mFormat(other.mFormat), mPixels(other.mPixels) {}
    
    PixelData& PixelData::operator=(const PixelData& other) {
        if (this != &other) {
            mFormat = other.mFormat;
            mPixels = other.mPixels;
        }
        return *this;
    }
    
    PixelData::PixelData(PixelData&& other) noexcept 
        : mFormat(other.mFormat), mPixels(std::move(other.mPixels)) {}
    
    PixelData& PixelData::operator=(PixelData&& other) noexcept {
        if (this != &other) {
            mFormat = other.mFormat;
            mPixels = std::move(other.mPixels);
        }
        return *this;
    }
    
    void PixelData::allocate(const uint8_t* data, size_t size) {
        if (data && size > 0) {
            mPixels.assign(data, data + size);
        }
    }
    
    void PixelData::allocate(size_t size) {
        mPixels.resize(size);
    }
    
    void PixelData::copyPixels(uint8_t* dest, size_t destSize) const {
        if (dest && destSize > 0) {
            size_t copySize = std::min(destSize, mPixels.size());
            std::memcpy(dest, mPixels.data(), copySize);
        }
    }
    
    void PixelData::copyFrom(const PixelData& other) {
        mFormat = other.mFormat;
        mPixels = other.mPixels;
    }
    
    void PixelData::append(const uint8_t* data, size_t size) {
        if (data && size > 0) {
            mPixels.insert(mPixels.end(), data, data + size);
        }
    }

} // face