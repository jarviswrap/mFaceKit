//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_TRACK_HPP
#define FACEDEMO_TRACK_HPP

#include "core/Component.hpp"
#include "core/clip/Clip.hpp"
#include "common/Duration.hpp"
#include "common/LoopThread.h"
#include <vector>

namespace face {

    enum class TrackType: uint8_t {
        Video, Audio, SubTitle
    };

    enum class TrackEvent: uint8_t {
        AddClip, RemoveClip
    };

    class Track: public std::enable_shared_from_this<Track>, public Component  {
    public:
        Track(TrackType type): mType(type), Component(ComponentType::Track, "") {};
        virtual ~Track();

        virtual bool requestRender(uint64_t timestamp);

        uint32_t addClip(std::shared_ptr<Clip> clip);
        void removeClip(uint32_t clipIndex);

        TrackType getType() const { return mType; }

        std::shared_ptr<Clip> getEndClip();
        uint64_t getEnd();

        void setListener(DataListener<TrackEvent, std::shared_ptr<Clip>> listener);

        const std::vector<std::shared_ptr<Clip>>& getAllClips() const { return mClips; }
    protected:
        TrackType mType{TrackType::Video};
        std::shared_ptr<Clip> mEndClip;
        std::vector<std::shared_ptr<Clip>> mClips;
        std::shared_ptr<LoopThread> mThread;
        std::mutex mMutex;
        DataListener <face::TrackEvent, std::shared_ptr<face::Clip>> mListener{nullptr};

        virtual void onThreadStart() {};
        virtual void onThreadStop() {};
        virtual void onRender(std::shared_ptr<Clip> clip, uint64_t timeStamp) = 0;
        std::shared_ptr<Clip> getCurrentClip(uint64_t timeStamp);
    };

} // face

#endif //FACEDEMO_TRACK_HPP
