//
// Created by wilbert on 2026/1/26.
//

#include "Track.hpp"
#include "common/Log.hpp"
#include <sstream>

namespace face {

    Track::~Track() {
        std::lock_guard<std::mutex> lk(mMutex);
        if (mThread) {
            mThread->stop();
            mThread = nullptr;
        }
    }

    uint32_t Track::addClip(std::shared_ptr <Clip> clip) {
        std::lock_guard<std::mutex> lk(mMutex);
        if (!clip || !clip->isValid()) {
            return 0;
        }
        if (!mEndClip || clip->getEnd() > mEndClip->getEnd()) {
            mEndClip = clip;
        }
        if (mListener) mListener(TrackEvent::AddClip, clip);
        mClips.push_back(clip);
        int size = mClips.size();
        if (!mThread) {
            std::stringstream trackName("Track");
            trackName << getId();
            mThread = std::make_shared<LoopThread>(trackName.str());
            mThread->setLoopMode(LoopMode::REQUEST);
            std::weak_ptr<Track> weakPtr(shared_from_this());
            mThread->setOnLoopListener([weakPtr] (uint64_t requestId) -> void {
                if (auto ptr = weakPtr.lock()) {
                    ptr->onRender(ptr->getCurrentClip(requestId), requestId);
                }
            });
            mThread->setOnStartListener([weakPtr] () -> void {
                if (auto ptr = weakPtr.lock()) {
                    ptr->onThreadStart();
                }
            });
            mThread->setOnStopListener([weakPtr] () -> void {
                if (auto ptr = weakPtr.lock()) {
                    ptr->onThreadStop();
                }
            });
            mThread->start();
        }
        return size;
    }

    std::shared_ptr <Clip> Track::getEndClip() {
        return mEndClip;
    }

    uint64_t Track::getEnd() {
        auto clip = mEndClip;
        return clip? clip->getEnd(): 0;
    }

    void Track::removeClip(uint32_t clipIndex) {
        std::lock_guard<std::mutex> lk(mMutex);
        int size = mClips.size();
        if (clipIndex >= size) {
            return;
        }
        if (mListener) mListener(TrackEvent::RemoveClip, mClips[clipIndex]);
        mClips.erase(mClips.begin() + clipIndex);
        if (mClips.size() == 0 && mThread) {
            mThread->stop();
            mThread = nullptr;
        }
    }

    std::shared_ptr <Clip> Track::getCurrentClip(uint64_t timeStamp) {
        std::lock_guard<std::mutex> lk(mMutex);
        std::shared_ptr<Clip> mActiveClip;
        for(auto& clip: mClips) {
            if ((!mActiveClip && clip->isActive(timeStamp)) || (clip->isActive(timeStamp) && mActiveClip->getClipType() != ClipType::Transition)) {
                mActiveClip = clip;
                if (clip->getClipType() == ClipType::Transition) {
                    return clip; // 转场优先级最高，发现转场直接返回
                }
            }
        }
        return mActiveClip;
    }

    bool Track::requestRender(uint64_t timestamp) {
        LOGE("Track::%s %llu, trackId:%d", __FUNCTION__, static_cast<unsigned long long>(timestamp), getId());
        auto thread = mThread;
        if (thread) {
            thread->requestLoop(timestamp);
            return true;
        }
        return false;
    }

    void Track::setListener(DataListener <face::TrackEvent, std::shared_ptr<face::Clip>> listener) {
        mListener = listener;
    }

} // face