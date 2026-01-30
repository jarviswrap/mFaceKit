//
// Created by wilbert on 2026/1/30.
//

#ifndef FACEDEMO_RENDER_HPP
#define FACEDEMO_RENDER_HPP

#include "common/Common.hpp"
#include "common/Size.hpp"
#include "common/RenderData.hpp"
#include "FrameBuffer.hpp"
#include "detect/Detector.hpp"

namespace face {
    template<typename T>
    class Render { // Render只在当前环境上直接绘制，不会包含FBO
    public:
        Render() = default;
        virtual ~Render() = default;
        Error render(const std::shared_ptr<T> &data, const std::shared_ptr<RenderTarget>& renderTarget) {
            if (!data) return Error::Err_InvalidInput;
            if (renderTarget) {
                auto targetWidth = renderTarget->getWidth();
                auto targetHeight = renderTarget->getHeight();
                if (mRenderTargetSize.setSize(targetWidth, targetHeight)) {
                    onRenderSizeChanged(targetWidth, targetHeight);
                }
                renderTarget->bind();
                int dataWidth = targetWidth;
                int dataHeight = targetHeight;
                if (getDataSize(data, dataWidth, dataHeight)) {
                    renderTarget->fitViewPort(dataWidth, dataHeight);
                }
            }
            return onRender(data, renderTarget?renderTarget->getRotation(): 0);
        }

        void destroy() {
            onDestroy();
        };

    protected:
        Size<int> mRenderTargetSize;
        virtual bool getDataSize(const std::shared_ptr<T> &data, int& width, int& height) = 0;
        virtual void onRenderSizeChanged(int width, int height) {};
        virtual Error onRender(const std::shared_ptr<T> &data, int rotation) = 0;
        virtual void onDestroy() {}
    };
} // face

#endif //FACEDEMO_RENDER_HPP
