//
// Created by wilbert on 2026/1/18.
//

#include "ImagePreviewer.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLSurfaceView.hpp"
#include "common/Log.hpp"

namespace face {
    ImagePreviewer::ImagePreviewer(std::shared_ptr<Processor<PixelData, PixelData>> processor): mProcessor(processor) {
        LOGE("ImagePreviewer::%s", __FUNCTION__ );
    }

    ImagePreviewer::~ImagePreviewer() {
        LOGE("ImagePreviewer::%s", __FUNCTION__ );
    }

    Error ImagePreviewer::start() {
        if (mSource) {
            return Error::Err_InvalidShowView;
        }
        LOGE("ImagePreviewer::%s", __FUNCTION__ );
        mSource = std::make_shared<ImageSource>();
        mImagePipeline = std::make_shared<Pipeline<PixelData, PixelData>>();
//        mAndroidDisplayDestination = std::make_shared<Destination<PixelData>>(EGLDelegate::getInstance().getShowView());
//        std::weak_ptr<Destination<PixelData>> destPtr(mAndroidDisplayDestination);
//        EGLDelegate::getInstance().setShowViewListener([destPtr] (const std::shared_ptr<EGLSurfaceView>& showView) -> void {
//            if (auto dest = destPtr.lock()) {
//                dest->setConsumer(showView);
//            }
//        });
//        return mImagePipeline->start(mSource, mAndroidDisplayDestination, mProcessor? mProcessor: std::make_shared<PassProcessor<PixelData>>());
        return Error::Err_InvalidShowView;
    }

    void ImagePreviewer::requestLoadImage(const std::string &imagePath) {
        LOGE("ImagePreviewer::%s, %s", __FUNCTION__, imagePath.c_str());
        mSource->requestLoadImage(imagePath);
    }

    Error ImagePreviewer::stop() {
        LOGE("ImagePreviewer::%s", __FUNCTION__ );
        return mImagePipeline->stop();
    }
} // face