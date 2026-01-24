//
// Created by wilbert on 2026/1/23.
//

#include "RetinaFace.hpp"
#include "common/Log.hpp"

namespace face {
    RetinaFace::RetinaFace(const std::string& modelPath) {
        mModelPath = modelPath;

        std::string pfldPath = "pfld.mnn";
        auto pos = modelPath.find_last_of("/\\");
        if (pos != std::string::npos) {
            pfldPath = modelPath.substr(0, pos + 1) + "pfld.mnn";
        }
        mLandmarker = std::make_shared<PFLDLandmarker>(pfldPath);
    }

    RetinaFace::~RetinaFace() {

    }

    std::shared_ptr<face::PixelData>
    RetinaFace::onProcess(const std::shared_ptr<face::PixelData> &input) {
        if (!input) return nullptr;
        if (initModel() != Error::None) return nullptr;
        if (initPreprocess(input->getFormat()) != Error::None) return nullptr;
        auto size = input->getResolution();
        
        // Calculate Letterbox parameters
        float src_w = (float)size.getWidth();
        float src_h = (float)size.getHeight();
        float dst_w = (float)_in_w;
        float dst_h = (float)_in_h;

        float scale = std::min(dst_w / src_w, dst_h / src_h);
        float nw = src_w * scale;
        float nh = src_h * scale;
        float dx = (dst_w - nw) * 0.5f;
        float dy = (dst_h - nh) * 0.5f;

        MNN::CV::Matrix trans;
        trans.postTranslate(-dx, -dy);
        trans.postScale(1.0f/scale, 1.0f/scale);
        mMNNProcessor->setMatrix(trans);
        
        // Fill background with black (optional, ImageProcess might not support setPadding explicitly in this version, 
        // but transform maps outside pixels. MNN::CV::ImageProcess::Config wrap? 
        // Default might be clamping or zero. Python does explicit fill. 
        // For MNN ImageProcess, pixels outside source are usually clamped or zero depending on wrap mode.
        // Let's assume clamping/zero is acceptable or handled.)
        
        mMNNProcessor->convert(input->getPixels(), size.getWidth(), size.getHeight(), 0, getInput().get());
        mInterpreter->runSession(mSession);
        
        MNN::Tensor *_output_cls_tensor = mInterpreter->getSessionOutput(mSession, "cls");
        MNN::Tensor *_output_bbox_tensor = mInterpreter->getSessionOutput(mSession, "bbox");
        MNN::Tensor *_output_ldmk_tensor = mInterpreter->getSessionOutput(mSession, "ldmk");

        // Copy to host tensors to ensure safe access and correct layout
        std::shared_ptr<MNN::Tensor> cls_host(new MNN::Tensor(_output_cls_tensor, MNN::Tensor::CAFFE));
        _output_cls_tensor->copyToHostTensor(cls_host.get());
        
        std::shared_ptr<MNN::Tensor> bbox_host(new MNN::Tensor(_output_bbox_tensor, MNN::Tensor::CAFFE));
        _output_bbox_tensor->copyToHostTensor(bbox_host.get());
        
        std::shared_ptr<MNN::Tensor> ldmk_host(new MNN::Tensor(_output_ldmk_tensor, MNN::Tensor::CAFFE));
        _output_ldmk_tensor->copyToHostTensor(ldmk_host.get());

        float *scores = cls_host->host<float>();
        float *offsets = bbox_host->host<float>();
        float *ldmks = ldmk_host->host<float>();
        
        std::vector<BBox> final_bboxes;
        for (const Box& anchor : anchors) {
            BBox bbox;
            Box refined_box;

            if (scores[1] > this->_score_threshold) {
                // score
                bbox.score = scores[1];

                // bbox
                refined_box.cx = anchor.cx + offsets[0] * 0.1 * anchor.sx;
                refined_box.cy = anchor.cy + offsets[1] * 0.1 * anchor.sy;
                refined_box.sx = anchor.sx * exp(offsets[2] * 0.2);
                refined_box.sy = anchor.sy * exp(offsets[3] * 0.2);

                // Convert from Normalized Tensor Coords to Tensor Pixel Coords
                float x1_t = (refined_box.cx - refined_box.sx/2) * dst_w;
                float y1_t = (refined_box.cy - refined_box.sy/2) * dst_h;
                float x2_t = (refined_box.cx + refined_box.sx/2) * dst_w;
                float y2_t = (refined_box.cy + refined_box.sy/2) * dst_h;
                
                // Map back to Source Image Coords (Inverse Letterbox)
                bbox.x1 = (x1_t - dx) / scale;
                bbox.y1 = (y1_t - dy) / scale;
                bbox.x2 = (x2_t - dx) / scale;
                bbox.y2 = (y2_t - dy) / scale;

                clip_bboxes(bbox, size.getWidth(), size.getHeight());

                // landmarks
                for (int i = 0; i < 5; i++) {
                    float lx_t = (anchor.cx + ldmks[2*i] * 0.1 * anchor.sx) * dst_w;
                    float ly_t = (anchor.cy + ldmks[2*i+1] * 0.1 * anchor.sy) * dst_h;
                    
                    bbox.landmarks[i].x = (lx_t - dx) / scale;
                    bbox.landmarks[i].y = (ly_t - dy) / scale;
                }
                final_bboxes.push_back(bbox);
            }

            scores += 2;
            offsets += 4;
            ldmks += 10;
        }

        std::sort(final_bboxes.begin(), final_bboxes.end(), [](BBox &lsh, BBox &rsh) {
            return lsh.score > rsh.score;
        });
        nms(final_bboxes, this->_nms_threshold);
        input->bboxes = final_bboxes;
        for(auto& bbox: final_bboxes) {
            LOGE("RetinaFaceBBox score:%f [%f,%f],[%f,%f], points{[%f,%f], [%f,%f], [%f,%f], [%f,%f], [%f,%f]}", bbox.score, bbox.x1, bbox.y1, bbox.x2, bbox.y2, bbox.landmarks[0].x, bbox.landmarks[0].y, bbox.landmarks[1].x, bbox.landmarks[1].y, bbox.landmarks[2].x, bbox.landmarks[2].y, bbox.landmarks[3].x, bbox.landmarks[3].y, bbox.landmarks[4].x, bbox.landmarks[4].y);
        }

        if (mLandmarker) {
            mLandmarker->onProcess(input);
        }

        return input;
    }

    Error RetinaFace::initModel() {
        if (mInterpreter) {
            return Error::None;
        }
        auto interpreter = MNN::Interpreter::createFromFile(mModelPath.c_str());
        if (interpreter) {
            mInterpreter.reset(interpreter);
            MNN::ScheduleConfig config;
            config.type = MNNForwardType::MNN_FORWARD_CPU;
            config.numThread = 1;
            mSession = mInterpreter->createSession(config);

            // resize session according input shape
            auto input = getInput();
            if (input) {
                mInterpreter->resizeTensor(input.get(), {1, 3, _in_h, _in_w});
                mInterpreter->resizeSession(mSession);
            }
            return Error::None;
        }
        return Error::Err_ModelInvalid;
    }

    Error RetinaFace::initPreprocess(PixelFormat format) {
        if (mMNNProcessor && mCurrentFormat == format) {
            return Error::None;
        }
        auto input = getInput();
        if (!input) return Error::Err_InvalidInput;
        MNN::CV::ImageProcess::Config preproc_config;
        preproc_config.filterType = MNN::CV::BILINEAR;
        memcpy(preproc_config.mean, this->_mean_vals, sizeof(this->_mean_vals));
        preproc_config.normal[0] = 1.0f;
        preproc_config.normal[1] = 1.0f;
        preproc_config.normal[2] = 1.0f;
        
        switch (format) {
            case PixelFormat::NV21:
            case PixelFormat::I420P:
                preproc_config.sourceFormat = MNN::CV::YUV_NV21;
                break;
            case PixelFormat::RGBA:
                preproc_config.sourceFormat = MNN::CV::RGBA;
                break;
            case PixelFormat::RGB:
                preproc_config.sourceFormat = MNN::CV::RGB;
                break;
            case PixelFormat::BGR:
                preproc_config.sourceFormat = MNN::CV::BGR;
                break;
            default:
                preproc_config.sourceFormat = MNN::CV::RGBA;
                break;
        }
        
        preproc_config.destFormat = MNN::CV::BGR;
        mMNNProcessor = std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(preproc_config));
        
        mCurrentFormat = format;

        this->create_anchors(anchors, _in_w, _in_h);
        return Error::None;
    }

    std::shared_ptr<MNN::Tensor> RetinaFace::getInput() {
        if (!mInputTensor) {
            auto tensor = mInterpreter->getSessionInput(mSession, "input");
            if (tensor) {
                mInputTensor.reset(tensor);
            }
        }
        return mInputTensor;
    }

    void RetinaFace::create_anchors(std::vector<Box> &anchors, int w, int h) const {
        anchors.clear();
        std::vector<std::vector<int> > feature_map(3), anchor_sizes(3);
        float strides[3] = {8, 16, 32};
        for (int i = 0; i < feature_map.size(); ++i) {
            feature_map[i].push_back(ceil(h/strides[i]));
            feature_map[i].push_back(ceil(w/strides[i]));
        }
        std::vector<int> stage1_size = {10, 20};
        anchor_sizes[0] = stage1_size;
        std::vector<int> stage2_size = {32, 64};
        anchor_sizes[1] = stage2_size;
        std::vector<int> stage3_size = {128, 256};
        anchor_sizes[2] = stage3_size;

        for (int k = 0; k < feature_map.size(); ++k) {
            std::vector<int> anchor_size = anchor_sizes[k];
            for (int i = 0; i < feature_map[k][0]; ++i) {
                for (int j = 0; j < feature_map[k][1]; ++j) {
                    for (int l = 0; l < anchor_size.size(); ++l) {
                        float kx = anchor_size[l]* 1.0 / w;
                        float ky = anchor_size[l]* 1.0 / h;
                        float cx = (j + 0.5) * strides[k] / w;
                        float cy = (i + 0.5) * strides[k] / h;
                        anchors.push_back({cx, cy, kx, ky});
                    }
                }
            }
        }
    }

    void RetinaFace::nms(std::vector<BBox> &bboxes, float nms_threshold) const {
        std::vector<float> bbox_areas(bboxes.size());
        for (int i = 0; i < bboxes.size(); i++) {
            bbox_areas[i] = (bboxes.at(i).x2 - bboxes.at(i).x1 + 1) * (bboxes.at(i).y2 - bboxes.at(i).y1 + 1);
        }

        for (int i = 0; i < bboxes.size(); i++) {
            for (int j = i + 1; j < bboxes.size(); ) {
                float xx1 = std::max(bboxes[i].x1, bboxes[j].x1);
                float yy1 = std::max(bboxes[i].y1, bboxes[j].y1);
                float xx2 = std::min(bboxes[i].x2, bboxes[j].x2);
                float yy2 = std::min(bboxes[i].y2, bboxes[j].y2);
                float w = std::max(float(0), xx2 - xx1 + 1);
                float h = std::max(float(0), yy2 - yy1 + 1);
                float inter = w * h;
                float IoU = inter / (bbox_areas[i] + bbox_areas[j] - inter);
                if (IoU >= nms_threshold) {
                    bboxes.erase(bboxes.begin() + j);
                    bbox_areas.erase(bbox_areas.begin() + j);
                } else {
                    j++;
                }
            }
        }
    }

    void RetinaFace::clip_bboxes(BBox &bbox, int w, int h) const {
        if(bbox.x1 < 0) bbox.x1 = 0;
        if(bbox.y1 < 0) bbox.y1 = 0;
        if(bbox.x2 > w) bbox.x2 = w;
        if(bbox.y2 > h) bbox.y2 = h;
    }
} // face