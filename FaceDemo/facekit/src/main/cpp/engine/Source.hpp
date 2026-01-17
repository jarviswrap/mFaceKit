//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_SOURCE_HPP
#define FACEDEMO_SOURCE_HPP

#include <utility>

#include "common/Size.hpp"
#include "common/PixelData.hpp"
#include "common/Common.hpp"

namespace face {
    class Source {
    public:
        virtual ~Source() = default;
        virtual std::shared_ptr<PixelData> getPixel() = 0;
        virtual Size<uint16_t> getSize() = 0;

        virtual bool isDataAvailable() { return false; };

        virtual void setAvailableListener(DataListener<Size<uint16_t>, PixelData> listener) {
            mDataListener = std::move(listener);
        };
    private:
        DataListener<Size<uint16_t>, PixelData> mDataListener;
    };
}
#endif //FACEDEMO_SOURCE_HPP
