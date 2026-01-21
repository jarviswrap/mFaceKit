//
// Created by wilbert on 2026/1/18.
//

#include "ImagePreviewer.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLSurfaceView.hpp"

namespace face {
    Error ImagePreviewer::start() {
        if (mSource) {
            return Error::Err_InvalidShowView;
        }
        mSource = std::make_shared<ImageSource>();
        mImagePipeline = std::make_shared<Pipeline<PixelData, PixelData>>();
        auto showView = EGLDelegate::getInstance().getShowView();
        if (!showView) {
            auto androidDisplayDestination = std::make_shared<Destination<PixelData>>(showView);
            return mImagePipeline->start(mSource, androidDisplayDestination, std::make_shared<PassProcessor<PixelData>>());
        }
        return Error::Err_InvalidShowView;
    }

    void ImagePreviewer::requestLoadImage(const std::string &imagePath) {
        mSource->requestLoadImage(imagePath);
    }

    Error ImagePreviewer::stop() {
        return mImagePipeline->stop();
    }
} // face