//
// Created by wilbert on 2026/1/16.
//

#include "ImageSource.hpp"
#include "../../include/cv/cv.hpp"

namespace face {
    ImageSource::ImageSource(const std::string &imagePath) {
        mThread = std::make_shared<LoopThread>();
        mThread->setLoopMode(LoopMode::REQUEST);
        mCurrentData = std::make_shared<PixelData>();
        std::weak_ptr<PixelData> weakPtr(mCurrentData);
        mThread->setOnLoopListener([weakPtr, this](uint32_t requestId) -> void {
            if (auto pixelData = weakPtr.lock()) {
                auto path = getImagePath(requestId - 1);
                if (!path.empty()) {
                    // 读取图像文件（默认BGR格式）
                    auto imageVar = MNN::CV::imread(path, MNN::CV::IMREAD_COLOR);
                    auto info = imageVar->getInfo();
                    if (info != nullptr) {
                        // 获取图像尺寸信息
                        auto &dims = info->dim;
                        if (dims.size() >= 3) {
                            int height = dims[0];
                            int width = dims[1];
                            int channels = dims[2];

                            // 读取像素数据
                            auto imagePtr = imageVar->readMap<uint8_t>();
                            if (imagePtr != nullptr) {
                                size_t dataSize = info->size;
                                pixelData->setPixelData((uint8_t *) imagePtr, width, height,
                                                        PixelFormat::BGR, dataSize);
                                if (mDataListener) {
                                    mDataListener(pixelData);
                                }
                                // 释放映射
                                imageVar->unMap();
                            }
                        }
                    }
                }
            }
        });
        mThread->start();
        requestLoadImage(imagePath);
    }

    ImageSource::~ImageSource() noexcept {
        mThread->stop();
    }

    void ImageSource::requestLoadImage(const std::string &imagePath) {
        auto size = mImagePathList.size();
        if (size > 0 && imagePath == mImagePathList[size - 1]) {
            return;
        }
        mImagePathList.push_back(imagePath);
        mThread->requestLoop(size + 1);
    }

    std::shared_ptr<PixelData> ImageSource::getNextData() {
        return mCurrentData;
    }

    std::string ImageSource::getImagePath(uint32_t index) {
        if (index >= mImagePathList.size()) return "";
        return mImagePathList[index];
    }

    bool ImageSource::isDataAvailable() {
        return mCurrentData && !mCurrentData->isEmpty();
    }

} // face