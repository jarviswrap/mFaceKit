//
// Created by wilbert on 2026/1/24.
//

#ifndef FACEDEMO_PFLDLANDMARKER_HPP
#define FACEDEMO_PFLDLANDMARKER_HPP
#include "Processor.hpp"
#include "common/PixelData.hpp"
#include "MNN/Interpreter.hpp"
#include "MNN/ImageProcess.hpp"

namespace face {

    class PFLDLandmarker: public Processor<PixelData, PixelData>  {
    public:
        PFLDLandmarker(const std::string& modelPath);
        ~PFLDLandmarker();

        std::shared_ptr<face::PixelData> onProcess(const std::shared_ptr<face::PixelData> &input) override;

    private:
        std::string mModelPath{""};
        std::shared_ptr<MNN::Interpreter> mInterpreter{nullptr};
        MNN::Session* mSession{ nullptr };
        std::shared_ptr<MNN::Tensor> mInputTensor;
        std::shared_ptr<MNN::CV::ImageProcess> mMNNProcessor;

        PixelFormat mCurrentFormat{PixelFormat::UNKNOWN};
        const int _in_w = 96;
        const int _in_h = 96;
        const float _mean_vals[3] = {123.0f, 123.0f, 123.0f};
        const float _norm_vals[3] = {0.01724f, 0.01724f, 0.01724f};

        int initModel();
        int initPreprocess(PixelFormat format);
        std::shared_ptr<MNN::Tensor> getInput();
    };

} // face

#endif //FACEDEMO_PFLDLANDMARKER_HPP
