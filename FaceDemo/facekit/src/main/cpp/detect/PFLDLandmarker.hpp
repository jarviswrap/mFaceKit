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

    /**
     * @brief PFLD (Practical Facial Landmark Detector) 人脸关键点检测器
     * 
     * 该类实现了基于 MNN 框架的 PFLD 模型调用，用于检测人脸的 98 个关键点。
     * 输入为包含人脸框的图像数据，输出为填充了关键点信息的图像数据。
     */
    class PFLDLandmarker: public Processor<PixelData, PixelData>  {
    public:
        /**
         * @brief 构造函数
         * @param modelPath PFLD MNN 模型文件的绝对路径
         */
        PFLDLandmarker(const std::string& modelPath);
        ~PFLDLandmarker();

        /**
         * @brief 执行关键点检测
         * 
         * 对输入 PixelData 中的每一张人脸（由 bboxes 定义）进行裁剪、预处理，
         * 并运行 PFLD 模型推断出 98 个关键点坐标，结果回填至 bbox.keypoints 中。
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
        // PFLD 模型输入尺寸 96x96
        const int _in_w = 96;
        const int _in_h = 96;
        // 归一化参数 mean: 123.0, norm: 1/58.0 approx 0.01724
        const float _mean_vals[3] = {123.0f, 123.0f, 123.0f};
        const float _norm_vals[3] = {0.01724f, 0.01724f, 0.01724f};

        /**
         * @brief 初始化 MNN 模型解释器和会话
         * @return 0 表示成功，非 0 表示失败
         */
        int initModel();

        /**
         * @brief 初始化图像预处理器 (MNN::CV::ImageProcess)
         * 根据输入图像格式配置颜色空间转换和归一化参数。
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

#endif //FACEDEMO_PFLDLANDMARKER_HPP
