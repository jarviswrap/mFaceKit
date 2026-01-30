//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_COMPOSER_HPP
#define FACEDEMO_COMPOSER_HPP
#include "io/Source.hpp"
#include "common/Rect.hpp"
#include "common/Duration.hpp"
#include "common/Position.hpp"
#include "core/track/Track.hpp"
#include "core/Component.hpp"

#include <unordered_map>

namespace face {

    enum class TickType: uint8_t{
        TickFromSource, TickInternal
    };

    class Timeline;
    class EGLSurfaceView;
    class TextureRender;
    class FrameBuffer;
    class VideoTrack;

    class Composer :public std::enable_shared_from_this<Composer>{
    public:
        Composer();
        ~Composer();

        void init(std::shared_ptr<Timeline> timeline = nullptr);
        uint32_t addTrack(Rect<float> normalizedRect, std::shared_ptr<Track> track);
        void bringTrackToLast(uint32_t trackId);
        void updateTrackRect(uint32_t trackId, float startXNormalized, float startYNormalized, float widthNormalized, float heightNormalized);

        template<typename T>
        std::shared_ptr<T> getComponentById(uint32_t componentId) {
            std::lock_guard<std::mutex> lk(mComponentMutex);
            auto it = mComponentMap.find(componentId);
            if (it != mComponentMap.end()) {
                return std::dynamic_pointer_cast<T>((*it).second);
            }
            return nullptr;
        }

        uint32_t getTrackSize();

    protected:
        std::unordered_map<uint32_t, std::shared_ptr<Component>> mComponentMap;
        std::unordered_map<uint32_t, Rect<float>> mTrackRectMap;
        std::vector<std::shared_ptr<Track>> mTrackList;
        std::shared_ptr<Timeline> mTimeline{nullptr};
        std::shared_ptr<EGLSurfaceView> mShowView{nullptr};
        std::shared_ptr<TextureRender> mRender{nullptr};
        std::shared_ptr<RenderTarget> mRenderTarget{nullptr};
        uint32_t mWidth{0};
        uint32_t mHeight{0};
        std::mutex mMutex;
        std::mutex mComponentMutex;

        void addComponent(const std::shared_ptr<Component>& component);
        void removeComponent(const std::shared_ptr<Component>& component);

        void updateClipMap(const TrackEvent& event, const std::shared_ptr<Clip>& clip);
        void setShowView(std::shared_ptr<EGLSurfaceView> showView);
        void onTick(uint64_t timeStamp);

        std::shared_ptr<RenderTarget> createRenderTarget(uint32_t trackId);
    };

} // face

#endif //FACEDEMO_COMPOSER_HPP
