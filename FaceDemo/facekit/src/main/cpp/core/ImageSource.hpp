//
// Created by wilbert on 2026/1/16.
//

#ifndef FACEDEMO_IMAGESOURCE_HPP
#define FACEDEMO_IMAGESOURCE_HPP
#include "Source.hpp"
#include "common/LoopThread.h"
#include "common/ObserverbleData.hpp"
#include <vector>

namespace face
{

    class ImageSource: Source
    {

    public:
        explicit ImageSource(const std::string& imagePath);
        ~ImageSource() noexcept override;
        void requestLoadImage(const std::string& imagePath);
        const std::string& getImagePath(uint32_t index);
        std::shared_ptr<PixelData> getPixelData() override;
        Size<uint16_t> getSize() override;
        bool isDataAvailable() override;
    private:
        std::shared_ptr<LoopThread> mThread;
        std::shared_ptr<PixelData> mCurrentData;
        std::vector<std::string> mImagePathList;
    };

} // face

#endif //FACEDEMO_IMAGESOURCE_HPP
