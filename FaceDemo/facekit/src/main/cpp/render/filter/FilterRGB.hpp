#pragma once

#include "FilterBase.hpp"

namespace face {

    class FilterRGBPoint;

    class FilterRGB: public FilterBase {
    public:
        FilterRGB(bool enableColorTest = false, bool enableColorPoint = false);
        virtual ~FilterRGB();
        virtual void draw(const GLuint *textureId);

        virtual void onInit();
        virtual void onInitialized();

        void setOutputSize(int outputWidth,
                           int outputHeight);  // MUST be called before init() when enableColorTest = true
    private:
        void switchColor();
        void drawWithColor();

    private:
        GLint mUniformTexLoc{-1};
        GLint mUniformColorLoc{-1};
        GLint mUniformWidthLoc{-1};
        GLint mUniformHeightLoc{-1};

        int   mWidth{0};
        int   mHeight{0};
        bool  mEnableColorPoint = false;
        bool  mEnableColorTest  = false;
        bool  mColorSwitch      = true;
        float mColorTest[3]     = {0};

        FilterRGBPoint *mFilterPoint{nullptr};
    };
}  // namespace pipeline
