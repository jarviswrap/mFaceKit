//
// Created by wilbert on 2026/1/23.
//

#ifndef FACEDEMO_RETINAFACE_HPP
#define FACEDEMO_RETINAFACE_HPP
#include "Processor.hpp"
#include "common/PixelData.hpp"
#include "MNN/Interpreter.hpp"
#include "MNN/ImageProcess.hpp"
#include "common/FaceBox.hpp"
namespace face {


    class RetinaFace: public Processor<PixelData, PixelData> {
    public:
        RetinaFace(const std::string& modelPath);
        ~RetinaFace();

        std::shared_ptr<face::PixelData> onProcess(const std::shared_ptr<face::PixelData> &input) override;

    private:
        std::string mModelPath{""};
        std::unique_ptr<MNN::Interpreter> mInterpreter{nullptr};
        MNN::Session* mSession{ nullptr };
        std::shared_ptr<MNN::Tensor> mInputTensor;
        std::shared_ptr<MNN::CV::ImageProcess> mMNNProcessor;

        float _nms_threshold = 0.4;
        float _score_threshold = 0.6;
        const float _mean_vals[3] = {104.f, 117.f, 123.f};
        const int _in_w = 480;
        const int _in_h = 320;
        PixelFormat mCurrentFormat{PixelFormat::UNKNOWN};
        std::vector<Box> anchors;

        Error initModel();
        Error initPreprocess(PixelFormat format);
        std::shared_ptr<MNN::Tensor> getInput();
        void create_anchors(std::vector<Box>& anchors, int w, int h) const;
        void nms(std::vector<BBox>& input_bboxes, float nms_threshold=0.5) const;
        void clip_bboxes(BBox& bbox, int w, int h) const;
    };

} // face

#endif //FACEDEMO_RETINAFACE_HPP
