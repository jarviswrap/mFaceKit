//
// Created by wilbert on 2026/1/30.
//

#ifndef FACEDEMO_INSPECTIONRENDER_HPP
#define FACEDEMO_INSPECTIONRENDER_HPP
#include "render/Render.hpp"
#include "common/PixelData.hpp"

namespace face {

    template <typename T>
    class Inspector {
    public:
        virtual void inspect(const std::shared_ptr<T>& data) = 0;
    };

    template <typename T>
    class PixelInspector: public Inspector<PixelData>{
        virtual void inspect(const std::shared_ptr<PixelData>& data) = 0;
        virtual std::shared_ptr<T> getResult(const std::shared_ptr<PixelData>& data, uint32_t timeout) = 0;
    };

    template <typename RData, typename IData>
    class InspectionRender: Render<RData> {
    public:
        InspectionRender(const std::shared_ptr<PixelInspector<IData>>& inspector):mInspector(inspector) {};

        Error onRender(const std::shared_ptr<RData> &data, int rotation) override {
            std::shared_ptr<IData> inspectData = nullptr;
            if (mInspector) {
                inspectData = mInspector->getResult(data, 200);
            }
            return onRender(data, inspectData, rotation);
        }

        std::shared_ptr<PixelInspector<IData>> getInspector() { return mInspector; }
    protected:
        std::shared_ptr<PixelInspector<IData>> mInspector{nullptr};
        virtual Error onRender(const std::shared_ptr<RData> &renderData, const std::shared_ptr<IData> &inspectData, int rotation) = 0;
    };

} // face

#endif //FACEDEMO_INSPECTIONRENDER_HPP
