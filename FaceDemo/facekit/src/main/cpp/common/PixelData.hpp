//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_PIXELDATA_HPP
#define FACEDEMO_PIXELDATA_HPP
#include <vector>
#include <cstring>

namespace face
{
    enum class PixelFormat: uint8_t {
        I420P, NV21, ARGB, RGBA, RGB
    };

    class PixelData
    {
    public:
        PixelData();
        
        virtual ~PixelData() = default;

        explicit PixelData(PixelFormat format);
        
        PixelData(PixelFormat format, size_t capacity);
        
        PixelData(const PixelData& other);
        
        PixelData& operator=(const PixelData& other);
        
        PixelData(PixelData&& other) noexcept;
        
        PixelData& operator=(PixelData&& other) noexcept;
        
        void setFormat(PixelFormat format) {
            mFormat = format;
        }
        
        PixelFormat getFormat() const {
            return mFormat;
        }
        
        const char* getFormatString() const {
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
                default:
                    return "UNKNOWN";
            }
        }

        uint8_t getBytesPerPixel() const {
            switch (mFormat) {
                case PixelFormat::RGB:
                    return 3;
                case PixelFormat::ARGB:
                case PixelFormat::RGBA:
                    return 4;
                case PixelFormat::I420P:
                case PixelFormat::NV21:
                    return 1; // 平面格式，这里返回1
                default:
                    return 0;
            }
        }
        
        void allocate(const uint8_t* data, size_t size);
        
        void allocate(size_t size);
        
        void clear() {
            mPixels.clear();
        }

        void reset() {
            mPixels.clear();
        }

        void shrinkToFit() {
            mPixels.shrink_to_fit();
        }
        
        uint8_t* getPixels() {
            return mPixels.empty() ? nullptr : mPixels.data();
        }

        const uint8_t* getPixels() const {
            return mPixels.empty() ? nullptr : mPixels.data();
        }

        size_t getSize() const {
            return mPixels.size();
        }

        size_t getCapacity() const {
            return mPixels.capacity();
        }

        bool isEmpty() const {
            return mPixels.empty();
        }
 
        void reserve(size_t capacity) {
            mPixels.reserve(capacity);
        }

        uint8_t getPixelByte(size_t index) const {
            if (index < mPixels.size()) {
                return mPixels[index];
            }
            return 0;
        }

        void setPixelByte(size_t index, uint8_t value) {
            if (index < mPixels.size()) {
                mPixels[index] = value;
            }
        }

        void copyPixels(uint8_t* dest, size_t destSize) const;

        void copyFrom(const PixelData& other);

        void append(const uint8_t* data, size_t size);

        bool isValidFormat() const {
            return mFormat == PixelFormat::I420P ||
                   mFormat == PixelFormat::NV21 ||
                   mFormat == PixelFormat::ARGB ||
                   mFormat == PixelFormat::RGBA ||
                   mFormat == PixelFormat::RGB;
        }

        bool isPlanarFormat() const {
            return mFormat == PixelFormat::I420P || 
                   mFormat == PixelFormat::NV21;
        }

        bool hasAlpha() const {
            return mFormat == PixelFormat::ARGB || 
                   mFormat == PixelFormat::RGBA;
        }

        size_t calculateDataSize(uint32_t width, uint32_t height) const {
            if (width == 0 || height == 0) {
                return 0;
            }
            
            switch (mFormat) {
                case PixelFormat::RGB:
                    return width * height * 3;
                case PixelFormat::ARGB:
                case PixelFormat::RGBA:
                    return width * height * 4;
                case PixelFormat::NV21:
                    return width * height * 3 / 2;
                case PixelFormat::I420P:
                    return width * height * 3 / 2;
                default:
                    return 0;
            }
        }

        bool validateDataSize(uint32_t width, uint32_t height) const {
            return getSize() == calculateDataSize(width, height);
        }
        
    private:
        PixelFormat mFormat;
        std::vector<uint8_t> mPixels;
    };

} // face

#endif //FACEDEMO_PIXELDATA_HPP
