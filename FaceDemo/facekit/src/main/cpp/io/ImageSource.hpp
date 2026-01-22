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
        ImageSource();
        ~ImageSource() noexcept override;
        void requestLoadImage(const std::string& imagePath);
        std::string getImagePath(int32_t index = -1) const;
        std::shared_ptr<PixelData> getNextData() override;
        bool isDataAvailable() override;
    private:
        std::shared_ptr<LoopThread> mThread{nullptr};
        std::shared_ptr<PixelData> mCurrentData{nullptr};
        std::vector<std::string> mImagePathList;
    };

} // face

#endif //FACEDEMO_IMAGESOURCE_HPP
