//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_PIXELDATA_HPP
#define FACEDEMO_PIXELDATA_HPP
#include <vector>
#include <cstring>
#include "common/Size.hpp"
namespace face
{
    enum class PixelFormat: uint8_t {
        UNKNOWN, I420P, NV21, ARGB, RGBA, RGB, BGR
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
        /**
         * 比较两个PixelData对象是否相等
         * 比较格式、分辨率和像素数据
         */
        bool operator==(const PixelData& other) const;
        
        /**
         * 比较两个PixelData对象是否不相等
         */
        bool operator!=(const PixelData& other) const {
            return !(*this == other);
        }
        
        void setLabel(const std::string& label) {
            mLabel = label;
        }

        void setFormat(PixelFormat format) {
            mFormat = format;
        }
        
        PixelFormat getFormat() const {
            return mFormat;
        }
        
        const char* getFormatString() const;

        static float getBytesPerPixel(PixelFormat format);
        
        void allocate(const uint8_t* data, size_t size);
        
        void allocate(size_t size);
        
        void clear();

        const uint8_t* getPixels() const {
            return mPixels;
        }

        uint32_t getPixelSize() const {
            return mPixelSize;
        }

        uint32_t getCapacity() const {
            return mCapacity;
        }

        bool isEmpty() const {
            return mPixelSize == 0;
        }
 
        void reserve(size_t capacity);

        void setPixelData(uint8_t* pixel, uint32_t width, uint32_t height, PixelFormat format, uint32_t size = 0);

        uint8_t getPixelByte(size_t index) const;

        void setPixelByte(size_t index, uint8_t value);

        void copyPixels(uint8_t* dest, size_t destSize) const;

        void copyFrom(const PixelData& other);

        void append(const uint8_t* data, size_t size);

        bool hasAlpha() const {
            return mFormat == PixelFormat::ARGB || 
                   mFormat == PixelFormat::RGBA;
        }

        uint32_t calculateDataSize() const;

        bool validateDataSize() const;

        const Size<uint16_t>& getResolution() const { return mResolution; };
    private:
        PixelFormat mFormat{PixelFormat::I420P};
        uint8_t *mPixels{nullptr};
        uint32_t mPixelSize{0};
        uint32_t mCapacity{0};
        bool mNeedFreePixel{false};
        Size<uint16_t> mResolution;
        std::string mLabel{""};
    };

} // face

#endif //FACEDEMO_PIXELDATA_HPP
