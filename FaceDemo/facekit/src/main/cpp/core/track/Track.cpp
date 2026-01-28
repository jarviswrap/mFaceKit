//
// Created by wilbert on 2026/1/26.
//

#include "Track.hpp"

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
        mClips.push_back(clip);
        int size = mClips.size();
        if (!mThread) {
            mThread = std::make_shared<LoopThread>("Track");
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
        auto thread = mThread;
        if (thread) {
            thread->requestLoop(timestamp);
            return true;
        }
        return false;
    }

    void Track::onRender(std::shared_ptr<Clip> clip, uint64_t timeStamp) {
        clip->render(timeStamp);
    }

} // face