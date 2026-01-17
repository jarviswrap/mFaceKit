//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_FACEINFERENCE_HPP
#define FACEDEMO_FACEINFERENCE_HPP
#include "Source.hpp"
#include "common/LimitQueue.hpp"
#include "common/LoopThread.h"
#include "MNN/Interpreter.hpp"
#include "MNN/MNNForwardType.h"
#include "MNN/Tensor.hpp"
namespace face
{

    class FaceInference
    {
    public:
        FaceInference();
        ~FaceInference();
        Error init(const std::string& model, int forward = MNNForwardType::MNN_FORWARD_CPU, int numThread = 4);
        Error start(const std::shared_ptr<Source>& source);
        Error stop();
        Error release();
    private:
        uint32_t mRequestId{0};
        std::shared_ptr<Source> mSource;
        std::shared_ptr<PixelData> mCurrentPixel;
        std::shared_ptr<LimitQueue<std::shared_ptr<PixelData>>> mDataQueue;
        std::shared_ptr<LoopThread> mThread;
        MNN::Session* mSession{ nullptr };
        std::unique_ptr<MNN::Interpreter> mInterpreter;
        std::shared_ptr<MNN::Tensor> mInputTensor;
    };

} // engine

#endif //FACEDEMO_FACEINFERENCE_HPP
