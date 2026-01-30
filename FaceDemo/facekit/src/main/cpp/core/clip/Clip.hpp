//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_CLIP_HPP
#define FACEDEMO_CLIP_HPP

#include "common/Duration.hpp"
#include "core/Component.hpp"
#include "render/RenderTarget.hpp"

namespace face {

    enum class ClipType: uint8_t{
        Video, Audio, Transition
    };

    class Clip: public Component{
    public:
        Clip(ClipType type, std::shared_ptr<ComponentId> trackId): mType(type), mTrackId(trackId),
                                                                   Component(ComponentType::Clip, "") {}
        ClipType getClipType() { return mType; };
        void setDuration(uint64_t start, uint64_t end) { mDuration.set(start, end); }
        Duration getDuration() const { return mDuration; }
        bool isActive(uint64_t timeStamp) const { return mDuration.isActive(timeStamp); }
        bool isValid() { return mDuration.duration() > 0; }
        virtual bool render(uint64_t timeStamp,const std::shared_ptr<RenderTarget>& renderTarget) { return false; };
        virtual bool releaseRender() { return false; };
        uint64_t getStart() { return mDuration.start; }
        uint64_t getEnd() { return mDuration.end; }
        uint32_t getTrackId() const { return mTrackId? mTrackId->getId(): 0; }
    protected:
        Duration mDuration;
        ClipType mType{ClipType::Video};
        std::shared_ptr<ComponentId> mTrackId{nullptr};
    };

} // face

#endif //FACEDEMO_CLIP_HPP
