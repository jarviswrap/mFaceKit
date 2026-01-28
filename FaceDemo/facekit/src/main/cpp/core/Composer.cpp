//
// Created by wilbert on 2026/1/26.
//

#include "Composer.hpp"
#include "Timeline.hpp"
#include "android/egl/EGLDelegate.hpp"
#include "android/egl/EGLSurfaceView.hpp"
#include "render/TextureRender.hpp"
#include "core/track/VideoTrack.hpp"
#include "render/utils/OpenGLUtils.hpp"
#include "render/utils/FrameBuffer.hpp"
#include "common/Log.hpp"

namespace face {
    Composer::Composer() {}

    Composer::~Composer() {}

    void Composer::init(std::shared_ptr<Timeline> timeline) {
        LOGE("Composer::%s sTimeline:%p", __FUNCTION__ , timeline.get());
        mTimeline = timeline;
        if (!mTimeline) {
            mTimeline = std::make_shared<Timeline>();
        }
        std::weak_ptr<Composer> weakPtr(shared_from_this());
        mTimeline->addTickListener([weakPtr](uint64_t timeStamp) -> void {
            if (auto sp = weakPtr.lock())
            {
                sp->onTick(timeStamp);
            }
        });

        setShowView(EGLDelegate::getInstance().getShowView());
        EGLDelegate::getInstance().setShowViewListener([weakPtr] (const std::shared_ptr<EGLSurfaceView>& showView) {
            if (auto sp = weakPtr.lock())
            {
                sp->setShowView(showView);
            }
        });
    }

    uint32_t Composer::addTrack(Rect<float> position, std::shared_ptr<Track> track) {
        std::lock_guard<std::mutex> lk(mMutex);
        if (track->getType() == TrackType::Video) {
            auto videoTrack = std::static_pointer_cast<VideoTrack>(track);
            videoTrack->resize(mWidth, mHeight);
        }
        mTrackList.push_back(track);
        mTrackMap.emplace(std::make_pair(track, position));
        auto size = mTrackList.size();
        if (size == 1) {
            mTimeline->start();
        }
        return size;
    }

    void Composer::onTick(uint64_t timeStamp) {
        std::lock_guard<std::mutex> lk(mMutex);
        for (auto& track: mTrackList) {
            track->requestRender(timeStamp);
        }
        auto showView = mShowView;
        if (showView) {
            showView->requestDraw();
        }
    }

    void Composer::setShowView(std::shared_ptr<EGLSurfaceView> showView) {
        mShowView = showView;
        if (mShowView) {
            std::weak_ptr<Composer> weakPtr(shared_from_this());
            mShowView->setSurfaceListener([weakPtr] (uint32_t width, uint32_t height) {
                if (auto sp = weakPtr.lock()) {
                    sp->mWidth = width;
                    sp->mHeight = height;
                    if (sp->mRender) {
                        sp->mRender->destroy();
                    }
                    sp->mRender = std::make_shared<TextureRender>();
                    sp->mRender->resize(width, height);
                    for (auto& track: sp->mTrackList) {
                        if (track->getType() == TrackType::Video) {
                            uint32_t w = 0;
                            uint32_t h = 0;
                            auto it = sp->mTrackMap.find(track); //查找获得addTrack时定义的归一化Rect
                            if (it != sp->mTrackMap.end()) {
                                auto rect = (*it).second;
                                float _normalizeWidth = rect.width;
                                float _normalizeHeight = rect.height;
                                w = _normalizeWidth * sp->mWidth;
                                h = _normalizeHeight * sp->mHeight;
                            }
                            auto videoTrack = std::static_pointer_cast<VideoTrack>(track);
                            videoTrack->resize(width, height);
                        }
                    }
                }
            });

            mShowView->setDrawListener([weakPtr] () {
                if (auto sp = weakPtr.lock()) {
                    if (!sp->mRender) return;
                    std::lock_guard<std::mutex> lk(sp->mMutex);
                    for (auto& track: sp->mTrackList) {
                        if (track->getType() == TrackType::Video) {
                            auto videoTrack = std::static_pointer_cast<VideoTrack>(track);
                            auto frameBuffer = videoTrack->getCurrentFrameBuffer();
                            if (frameBuffer) {
                                auto renderData = std::make_shared<RenderData<Texture>>();
                                uint32_t x = 0;
                                uint32_t y = 0;
                                uint32_t width = 0;
                                uint32_t height = 0;
                                videoTrack->getSize(width, height);
                                auto it = sp->mTrackMap.find(track);
                                if (it != sp->mTrackMap.end()) {
                                    auto rect = (*it).second;
                                    float _normalizeX = rect.startX;
                                    float _normalizeY = rect.startY;
                                    x = _normalizeX * sp->mWidth;
                                    y = _normalizeY * sp->mHeight;
                                }
                                renderData->rect.set(x, y, width, height);
                                renderData->scaleType = ScaleType::FitCenter;
                                renderData->data = frameBuffer->getFboTexture();
                                LOGE("Composer::%s renderTrace startRender Texture:%d", __FUNCTION__, renderData->data->getTextureId());
                                sp->mRender->render(renderData);
                                OpenGLUtils::finish();
                                LOGE("Composer::%s renderTrace finishRender Texture:%d", __FUNCTION__, renderData->data->getTextureId());
                            } else {
                                LOGE("Composer::%s empty FrameBuffer", __FUNCTION__ );
                            }
                        }
                    }
                }
            });

            mShowView->setSurfaceDestroyListener([weakPtr] () {
                if (auto sp = weakPtr.lock()) {
                    if (sp->mRender) {
                        sp->mRender->destroy();
                        sp->mRender.reset();
                    }
                }
            });
        }
    }

    void Composer::updateRect(uint32_t trackIndex, Rect<float> rect) {
        auto track = getTrackByIndex(trackIndex);
        if (track) {
            std::lock_guard<std::mutex> lk(mMutex);
            auto it = mTrackMap.find(track);
            if (it != mTrackMap.end()) {
                it->second = rect;
            }
        }
    }

    std::shared_ptr<Track> Composer::getTrackByIndex(uint32_t trackIndex) {
        std::lock_guard<std::mutex> lk(mMutex);
        auto size = mTrackList.size();
        if (trackIndex >= size) return nullptr;
        return mTrackList[trackIndex];
    }

    uint32_t Composer::getTrackSize() {
        return mTrackList.size();
    }

} // face