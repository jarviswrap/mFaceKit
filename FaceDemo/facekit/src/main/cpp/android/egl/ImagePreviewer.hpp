//
// Created by wilbert on 2026/1/18.
//

#ifndef FACEDEMO_IMAGEPREVIEWER_HPP
#define FACEDEMO_IMAGEPREVIEWER_HPP

#include "core/Pipeline.hpp"
#include "common/PixelData.hpp"
#include "io/ImageSource.hpp"
#include "io/Destination.hpp"

namespace face {

    class ImagePreviewer{
    public:
        ImagePreviewer();
        ~ImagePreviewer();
        Error start();
        void requestLoadImage(const std::string& imagePath);
        Error stop();
    private:
        std::shared_ptr<Pipeline<PixelData, PixelData>> mImagePipeline{nullptr};
        std::shared_ptr<ImageSource> mSource{nullptr};
        std::shared_ptr<Destination<PixelData>> mAndroidDisplayDestination;
    };

} // face

#endif //FACEDEMO_IMAGEPREVIEWER_HPP
