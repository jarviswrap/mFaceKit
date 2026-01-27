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

#include <unordered_map>

namespace face {

    enum class TickType: uint8_t{
        TickFromSource, TickInternal
    };

    class Timeline;
    class EGLSurfaceView;
    class TextureRender;
    class Composer :public std::enable_shared_from_this<Composer>{
    public:
        Composer();
        ~Composer();


        void init(std::shared_ptr<Timeline> timeline = nullptr);
        uint32_t addTrack(Rect<float> rect, std::shared_ptr<Track> track);
        void updateRect(uint32_t trackIndex, Rect<float> rect);

        std::shared_ptr<Track> getTrackByIndex(uint32_t trackPtr);
        uint32_t getTrackSize();
    protected:
        std::unordered_map<std::shared_ptr<Track>, Rect<float>> mTrackMap;
        std::vector<std::shared_ptr<Track>> mTrackList;
        std::shared_ptr<Timeline> mTimeline{nullptr};
        std::shared_ptr<EGLSurfaceView> mShowView{nullptr};
        std::shared_ptr<TextureRender> mRender{nullptr};
        uint32_t mWidth{720};
        uint32_t mHeight{1280};
        std::mutex mMutex;


        void setShowView(std::shared_ptr<EGLSurfaceView> showView);
        void onTick(uint64_t timeStamp);
    };

} // face

#endif //FACEDEMO_COMPOSER_HPP
