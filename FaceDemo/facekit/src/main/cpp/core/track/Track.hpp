//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_TRACK_HPP
#define FACEDEMO_TRACK_HPP

#include "core/clip/Clip.hpp"
#include "common/Duration.hpp"
#include "common/LoopThread.h"
#include <vector>

namespace face {

    enum class TrackType: uint8_t {
        Video, Audio, SubTitle
    };

    class Track: public std::enable_shared_from_this<Track>  {
    public:
        Track(TrackType type): mType(type) {};
        Track() = default;
        virtual ~Track();

        virtual bool requestRender(uint64_t timestamp);

        uint32_t addClip(std::shared_ptr<Clip> clip);
        void removeClip(uint32_t clipIndex);

        TrackType getType() const { return mType; }

        std::shared_ptr<Clip> getEndClip();
        uint64_t getEnd();
    protected:
        TrackType mType{TrackType::Video};
        std::shared_ptr<Clip> mEndClip;
        std::vector<std::shared_ptr<Clip>> mClips;
        std::shared_ptr<LoopThread> mThread;
        std::mutex mMutex;
        virtual void onThreadStart() {};
        virtual void onThreadStop() {};
        virtual void onRender(std::shared_ptr<Clip> clip, uint64_t timeStamp);

        std::shared_ptr<Clip> getCurrentClip(uint64_t timeStamp);
    };

} // face

#endif //FACEDEMO_TRACK_HPP
