//
// Created by wilbert on 2026/1/30.
//
#include <GLES/gl.h>
#include "RenderTarget.hpp"
#include "common/Log.hpp"

namespace face {

    bool RenderTarget::init(int width, int height)  {
        return mTargetRect.set(width, height);
    }

    bool RenderTarget::setPosition(int x, int y) {
        return mTargetRect.setPosition(x, y);
    }

    void RenderTarget::fitViewPort(int imageWidth, int imageHeight) {
        auto scaleType = mScaleType;
        auto x = 0;
        auto y = 0;
        auto width = imageWidth;
        auto height = imageHeight;
        mTargetRect.get(x, y, width, height);
        if (scaleType == ScaleType::Exactly) {
            glViewport(x, y, width, height);
            return;
        }
        width = getWidth();
        height = getHeight();
        auto viewPortWidth = width;
        auto viewPortHeight = height;

        float viewAspect = (float)width / height;
        float imageAspect = (float)imageWidth / imageHeight;

        if (scaleType == ScaleType::FitCenter) {
            if (imageAspect > viewAspect) {
                // Image is wider, fit width, black bars top/bottom
                // vw = mWidth;
                viewPortHeight = (int)(width / imageAspect);
                y = (height - viewPortHeight) / 2;
            } else {
                // Image is taller, fit height, black bars left/right
                // vh = mHeight;
                viewPortWidth = (int)(height * imageAspect);
                x = (width - viewPortWidth) / 2;
            }
        } else if (scaleType == ScaleType::CenterCrop) {
            if (imageAspect > viewAspect) {
                // Image is wider, crop width -> fill height
                // vh = mHeight;
                viewPortWidth = (int)(height * imageAspect);
                x = (width - viewPortWidth) / 2; // x will be negative
            } else {
                // Image is taller, crop height -> fill width
                // vw = mWidth;
                viewPortHeight = (int)(width / imageAspect);
                y = (height - viewPortHeight) / 2; // y will be negative
            }
        }
        //FitXY: use full mWidth, mHeight (default)
        LOGE("RenderTarget::%s dataSize:[%d,%d], viewPort[%d,%d,%d,%d]", __FUNCTION__, imageWidth, imageHeight, x, y, viewPortWidth, viewPortHeight);
        glViewport(x, y, viewPortWidth, viewPortHeight);
    }

    void RenderTarget::copyTo(const std::shared_ptr<RenderTarget> &target) {
        if (!target) return;
        target->setScaleType(getScaleType());
        target->setPosition(mTargetRect.startX, mTargetRect.startY);
        target->setRotation(getRotation());
        target->init(getWidth(), getHeight());
    }

} // face