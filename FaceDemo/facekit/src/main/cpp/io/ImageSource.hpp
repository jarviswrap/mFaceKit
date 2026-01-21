//
// Created by wilbert on 2026/1/16.
//

#ifndef FACEDEMO_IMAGESOURCE_HPP
#define FACEDEMO_IMAGESOURCE_HPP
#include "Source.hpp"
#include "common/PixelData.hpp"
#include "common/LoopThread.h"
#include <vector>

namespace face
{

    class ImageSource: public Source<PixelData>
    {

    public:
        explicit ImageSource();
        ~ImageSource() noexcept override;
        void requestLoadImage(const std::string& imagePath);
        std::string getImagePath(uint32_t index) const;
        std::shared_ptr<PixelData> getNextData() override;
        bool isDataAvailable() override;
    private:
        std::shared_ptr<LoopThread> mThread;
        std::shared_ptr<PixelData> mCurrentData;
        std::vector<std::string> mImagePathList;
    };

} // face

#endif //FACEDEMO_IMAGESOURCE_HPP
