//
// Created by wilbert on 2026/1/18.
//

#ifndef FACEDEMO_IMAGEPIPELINE_HPP
#define FACEDEMO_IMAGEPIPELINE_HPP

#include "Pipeline.hpp"
#include "common/PixelData.hpp"
#include "common/LimitQueue.hpp"
#include "common/LoopThread.h"

namespace face {

    class ImagePipeline: public Pipeline<PixelData, PixelData> {
    public:
        ImagePipeline();
        ~ImagePipeline();

    protected:
        Error onStart() override;
        Error onStop() override;

    private:
        std::shared_ptr<LoopThread> mThread;
        std::shared_ptr<LimitQueue<std::shared_ptr<PixelData>>> mDataQueue;
    };

} // face

#endif //FACEDEMO_IMAGEPIPELINE_HPP
