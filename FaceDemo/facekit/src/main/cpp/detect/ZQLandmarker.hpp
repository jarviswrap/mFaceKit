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

    /**
     * @brief ZQ Landmark 人脸关键点检测器
     * 
     * 该类实现了基于 MNN 框架的 ZQ Landmark 模型调用，用于检测人脸的 106 个关键点。
     * 相比 PFLD，ZQ 模型通常提供更密集的关键点分布，适用于更精细的面部特效。
     */
    class ZQLandmarker: public Processor<PixelData, PixelData>  {
    public:
        /**
         * @brief 构造函数
         * @param modelPath ZQ Landmark MNN 模型文件的绝对路径
         */
        ZQLandmarker(const std::string& modelPath);
        ~ZQLandmarker();

        /**
         * @brief 执行关键点检测
         * 
         * 对输入 PixelData 中的每一张人脸进行处理：
         * 1. 根据 bbox 计算方形裁剪区域（包含 padding）。
         * 2. 预处理并运行 ZQ 模型，推断出 106 个关键点。
         * 3. 将关键点坐标映射回原图坐标系并存储在 bbox.keypoints 中。
         * 
         * @param input 输入图像数据，必须包含检测到的人脸框 (bboxes)
         * @return 处理后的图像数据（包含关键点信息）
         */
        std::shared_ptr<face::PixelData> onProcess(const std::shared_ptr<face::PixelData> &input) override;

    private:
        std::string mModelPath{""};
        std::shared_ptr<MNN::Interpreter> mInterpreter{nullptr};
        MNN::Session* mSession{ nullptr };
        std::shared_ptr<MNN::Tensor> mInputTensor;
        std::shared_ptr<MNN::CV::ImageProcess> mMNNProcessor;

        PixelFormat mCurrentFormat{PixelFormat::UNKNOWN};
        // ZQ 模型输入尺寸 112x112
        const int _in_w = 112;
        const int _in_h = 112;
        // 归一化参数 mean: 127.5, norm: 1/128.0 approx 0.0078125
        const float _mean_vals[3] = {127.5f, 127.5f, 127.5f};
        const float _norm_vals[3] = {0.0078125f, 0.0078125f, 0.0078125f};

        /**
         * @brief 初始化 MNN 模型解释器和会话
         * @return 0 表示成功，非 0 表示失败
         */
        int initModel();

        /**
         * @brief 初始化图像预处理器
         * @param format 输入图像的像素格式
         * @return 0 表示成功
         */
        int initPreprocess(PixelFormat format);

        /**
         * @brief 获取模型输入张量
         * @return 输入张量指针
         */
        std::shared_ptr<MNN::Tensor> getInput();
    };

} // face

#endif //FACEDEMO_ZQLANDMARKER_HPP
