//
// Created by wilbert on 2026/1/24.
//

#ifndef FACEDEMO_ZQLANDMARKER_HPP
#define FACEDEMO_ZQLANDMARKER_HPP
#include "Processor.hpp"
#include "common/PixelData.hpp"
#include "MNN/Interpreter.hpp"
#include "MNN/ImageProcess.hpp"

namespace face {

    class ZQLandmarker: public Processor<PixelData, PixelData>  {
    public:
        ZQLandmarker(const std::string& modelPath);
        ~ZQLandmarker();

        std::shared_ptr<face::PixelData> onProcess(const std::shared_ptr<face::PixelData> &input) override;

    private:
        std::string mModelPath{""};
        std::shared_ptr<MNN::Interpreter> mInterpreter{nullptr};
        MNN::Session* mSession{ nullptr };
        std::shared_ptr<MNN::Tensor> mInputTensor;
        std::shared_ptr<MNN::CV::ImageProcess> mMNNProcessor;

        PixelFormat mCurrentFormat{PixelFormat::UNKNOWN};
        const int _in_w = 112;
        const int _in_h = 112;
        const float _mean_vals[3] = {127.5f, 127.5f, 127.5f};
        const float _norm_vals[3] = {0.0078125f, 0.0078125f, 0.0078125f};

        int initModel();
        int initPreprocess(PixelFormat format);
        std::shared_ptr<MNN::Tensor> getInput();
    };

} // face

#endif //FACEDEMO_ZQLANDMARKER_HPP
