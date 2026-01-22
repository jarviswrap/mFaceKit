//
// Created by wilbert on 2026/1/18.
//

#include "ImagePreviewer.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLSurfaceView.hpp"
#include "common/Log.hpp"

namespace face {
    ImagePreviewer::ImagePreviewer() = default;

    ImagePreviewer::~ImagePreviewer() = default;

    Error ImagePreviewer::start() {
        if (mSource) {
            return Error::Err_InvalidShowView;
        }
        mSource = std::make_shared<ImageSource>();
        mImagePipeline = std::make_shared<Pipeline<PixelData, PixelData>>();
        mAndroidDisplayDestination = std::make_shared<Destination<PixelData>>(EGLDelegate::getInstance().getShowView());
        std::weak_ptr<Destination<PixelData>> destPtr(mAndroidDisplayDestination);
        EGLDelegate::getInstance().setShowViewListener([destPtr] (const std::shared_ptr<EGLSurfaceView>& showView) -> void {
            if (auto dest = destPtr.lock()) {
                dest->setConsumer(showView);
            }
        });
        return mImagePipeline->start(mSource, mAndroidDisplayDestination, std::make_shared<PassProcessor<PixelData>>());
        return Error::Err_InvalidShowView;
    }

    void ImagePreviewer::requestLoadImage(const std::string &imagePath) {
        LOGE("ImagePreviewer::%s", __FUNCTION__ );
        mSource->requestLoadImage(imagePath);
    }

    Error ImagePreviewer::stop() {
        return mImagePipeline->stop();
    }
} // face