//
// Created by wilbert on 2026/1/30.
//

#ifndef FACEDEMO_RENDERTARGET_HPP
#define FACEDEMO_RENDERTARGET_HPP
#include "common/Rect.hpp"
#include <memory>

namespace face {
    enum class ScaleType {
        FitXY,
        CenterCrop,
        FitCenter,
        Exactly,
    };

    class RenderTarget {
    public:
        RenderTarget() = default;
        RenderTarget(int x, int y, int width, int height): mTargetRect(x, y, width, height), mScaleType(ScaleType::Exactly) {};
        virtual ~RenderTarget() = default;
        virtual int getWidth() { return mTargetRect.width; };
        virtual int getHeight() { return mTargetRect.height; };

        virtual bool init(int width, int height);
        virtual bool bind() { return false; }
        virtual void release() {};

        void setRotation(int rotation) { mRotation = rotation; }
        int getRotation() const { return mRotation; }

        void setScaleType(ScaleType scaleType) { mScaleType = scaleType; }
        ScaleType getScaleType() { return mScaleType; }
        bool setPosition(int x, int y);

        void fitViewPort(int dataWidth, int dataHeight);

        virtual void copyTo(const std::shared_ptr<RenderTarget>& target);
    protected:
        Rect<int> mTargetRect;
        ScaleType mScaleType{ScaleType::FitXY};

        int mRotation{0};
    };

} // face

#endif //FACEDEMO_RENDERTARGET_HPP
