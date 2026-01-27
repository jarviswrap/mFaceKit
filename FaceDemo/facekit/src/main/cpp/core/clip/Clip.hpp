//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_CLIP_HPP
#define FACEDEMO_CLIP_HPP

#include "common/Duration.hpp"

namespace face {

    enum class ClipType: uint8_t{
        Video, Audio, Transition
    };

    class Clip {
    public:
        ClipType getClipType() { return mType; };
        Duration getDuration() const { return mDuration; }
        bool isActive(uint64_t timeStamp) const { return mDuration.isActive(timeStamp); }
        bool isValid() { return mDuration.duration() > 0; }
        virtual bool render(uint64_t timeStamp) { return false; };
    private:
        Duration mDuration;
        ClipType mType{ClipType::Video};
    };

} // face

#endif //FACEDEMO_CLIP_HPP
