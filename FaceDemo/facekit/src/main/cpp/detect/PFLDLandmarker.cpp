//
// Created by wilbert on 2026/1/24.
//

#include "PFLDLandmarker.hpp"
#include "common/Log.hpp"
#include <sstream>

namespace face {
    PFLDLandmarker::PFLDLandmarker(const std::string &modelPath) {
        mModelPath = modelPath;
    }

    PFLDLandmarker::~PFLDLandmarker() {

    }

    std::shared_ptr<face::PixelData>
    PFLDLandmarker::onProcess(const std::shared_ptr<face::PixelData> &input) {
        if (!input) return nullptr;
        if (input->bboxes.empty()) return input;

        if (initModel() != 0) {
            LOGE("PFLD initModel failed");
            return nullptr;
        }
        if (initPreprocess(input->getFormat()) != 0) {
            LOGE("PFLD initPreprocess failed");
            return nullptr;
        }

        auto size = input->getResolution();
        int src_w = size.getWidth();
        int src_h = size.getHeight();

        for (auto& bbox : input->bboxes) {
            float x = bbox.x1;
            float y = bbox.y1;
            float w = bbox.x2 - bbox.x1;
            float h = bbox.y2 - bbox.y1;

            if (x < 0) x = 0;
            if (y < 0) y = 0;
            if (x + w > src_w) w = src_w - x;
            if (y + h > src_h) h = src_h - y;

            if (w <= 0 || h <= 0) continue;

            // Setup Matrix for Dest(0..96) -> Src(x..x+w)
            MNN::CV::Matrix trans;
            // Dest -> Src: Scale then Translate
            trans.postScale(w / (float)_in_w, h / (float)_in_h);
            trans.postTranslate(x, y);

            mMNNProcessor->setMatrix(trans);

            // Convert and resize
            mMNNProcessor->convert(input->getPixels(), src_w, src_h, 0, getInput().get());

            // Run
            mInterpreter->runSession(mSession);

            // Get output
            auto output = mInterpreter->getSessionOutput(mSession, "conv5_fwd");
            std::shared_ptr<MNN::Tensor> output_host(new MNN::Tensor(output, MNN::Tensor::CAFFE));
            output->copyToHostTensor(output_host.get());

            float* data = output_host->host<float>();

            bbox.keypoints.clear();
            for (int i = 0; i < 98; ++i) {
                float px = data[2 * i];
                float py = data[2 * i + 1];

                Point pt;
                pt.x = px * (w / (float)_in_w) + x;
                pt.y = py * (h / (float)_in_h) + y;
                bbox.keypoints.push_back(pt);
            }

            // Log first 10 points
            std::stringstream log_msg("PFLD Points: ");

            for (int i = 0; i < 10 && i < bbox.keypoints.size(); ++i) {
                log_msg << "[" << bbox.keypoints[i].x << "," << bbox.keypoints[i].y << "] ";
            }
            LOGE("%s", log_msg.str().c_str());
        }
        return input;
    }

    int PFLDLandmarker::initModel() {
        if (mInterpreter) return 0;
        mInterpreter = std::unique_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mModelPath.c_str()));
        if (!mInterpreter) return 10000;

        MNN::ScheduleConfig config;
        config.type = MNN_FORWARD_CPU;
        config.numThread = 1;
        MNN::BackendConfig backend_config;
        backend_config.memory    = MNN::BackendConfig::Memory_Normal;
        backend_config.power     = MNN::BackendConfig::Power_Normal;
        backend_config.precision = MNN::BackendConfig::Precision_Normal;
        config.backendConfig = &backend_config;
        mSession = mInterpreter->createSession(config);

        auto input = getInput();
        if (input) {
            mInterpreter->resizeTensor(input.get(), {1, 3, _in_h, _in_w});
            mInterpreter->resizeSession(mSession);
        }
        return 0;
    }

    int PFLDLandmarker::initPreprocess(PixelFormat format) {
        if (mMNNProcessor && mCurrentFormat == format) return 0;

        MNN::CV::ImageProcess::Config config;
        config.filterType = MNN::CV::BICUBIC;
        ::memcpy(config.mean, _mean_vals, sizeof(_mean_vals));
        ::memcpy(config.normal, _norm_vals, sizeof(_norm_vals));
        
        switch (format) {
            case PixelFormat::NV21:
            case PixelFormat::I420P:
                config.sourceFormat = MNN::CV::YUV_NV21;
                break;
            case PixelFormat::RGBA:
                config.sourceFormat = MNN::CV::RGBA;
                break;
            case PixelFormat::RGB:
                config.sourceFormat = MNN::CV::RGB;
                break;
            case PixelFormat::BGR:
                config.sourceFormat = MNN::CV::BGR;
                break;
            default:
                config.sourceFormat = MNN::CV::RGBA;
                break;
        }
        config.destFormat = MNN::CV::RGB;

        mMNNProcessor = std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(config));
        mCurrentFormat = format;
        return 0;
    }

    std::shared_ptr<MNN::Tensor> PFLDLandmarker::getInput() {
        if (!mInputTensor) {
            // Use nullptr to get the first input
            auto tensor = mInterpreter->getSessionInput(mSession, nullptr);
            if (tensor) {
                mInputTensor.reset(tensor);
            }
        }
        return mInputTensor;
    }

} // face
