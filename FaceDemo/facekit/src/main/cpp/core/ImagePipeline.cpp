//
// Created by wilbert on 2026/1/18.
//

#include "ImagePipeline.hpp"

namespace face {

    ImagePipeline::ImagePipeline() {
        mDataQueue = std::make_shared<LimitQueue<std::shared_ptr<PixelData>>>();
        mDataQueue->setMaxSize(2);
        mDataQueue->setPushListener()
    }

    ImagePipeline::~ImagePipeline() {

    }

    Error ImagePipeline::onStart() {

    }

    Error ImagePipeline::onStop() {

    }


} // face