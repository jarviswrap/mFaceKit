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
#include "render/FrameBuffer.hpp"
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

    uint32_t Composer::addTrack(Rect<float> normalizedRect, std::shared_ptr<Track> track) {
        if (!track) return 0;
        std::lock_guard<std::mutex> lk(mMutex);
        auto trackId = track->getId();
        mTrackRectMap[trackId] = normalizedRect;
        if (track->getType() == TrackType::Video && mWidth > 0 && mHeight > 0) {
            auto videoTrack = std::static_pointer_cast<VideoTrack>(track);
            videoTrack->setRenderTarget(createRenderTarget(trackId));
        }
        mTrackList.push_back(track);
        auto& clipList = track->getAllClips();
        for(auto& clip: clipList) { // 如果track中已经有clip，先将clip全部添加到mComponentMap
            addComponent(clip);
        }
        std::weak_ptr<Composer> weakSelf = shared_from_this();
        track->setListener([weakSelf](const TrackEvent& event, const std::shared_ptr<Clip>& clip) {
            if (auto self = weakSelf.lock()) {
                self->updateClipMap(event, clip);
            }
        });
        addComponent(track);

        if (mTrackList.size() == 1) {
            mTimeline->start();
        }
        LOGE("Composer::%s success, trackId:%d", __FUNCTION__, trackId);
        return trackId;
    }

    void Composer::bringTrackToLast(uint32_t trackId) {
        std::lock_guard<std::mutex> lk(mMutex);
        auto it = std::find_if(mTrackList.begin(), mTrackList.end(), [trackId](const std::shared_ptr<Track>& track) {
            return track->getId() == trackId;
        });
        if (it != mTrackList.end()) {
            auto track = *it;
            mTrackList.erase(it);
            mTrackList.push_back(track);
        }
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
                    std::lock_guard<std::mutex> lk(sp->mMutex);
                    sp->mWidth = width;
                    sp->mHeight = height;
                    if (sp->mRender) {
                        sp->mRender->destroy();
                    }
                    sp->mRender = std::make_shared<TextureRender>();
                    for (auto& track: sp->mTrackList) {
                        if (track->getType() == TrackType::Video) {
                            auto videoTrack = std::static_pointer_cast<VideoTrack>(track);
                            videoTrack->setRenderTarget(sp->createRenderTarget(videoTrack->getId()));
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
                                auto renderData = frameBuffer->getFboTexture();
//                                auto& rect = renderData->rect;
//                                sp->readNormalizedSizeByTrackId(track->getId(), rect.startX, rect.startY, rect.width, rect.height);
//                                renderData->scaleType = ScaleType::FitCenter;
//                                renderData->data = frameBuffer->getFboTexture();
                                LOGE("Composer::%s renderTrace startRender Texture:%d, track:%d", __FUNCTION__, renderData->getTextureId(), track->getId());
                                sp->mRender->render(renderData, videoTrack->getRenderTarget());
                                OpenGLUtils::finish();
                                //LOGE("Composer::%s renderTrace finishRender Texture:%d", __FUNCTION__, renderData->data->getTextureId());
                            } else {
                                LOGE("Composer::%s empty FrameBuffer, track:%d", __FUNCTION__, track->getId());
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

    void Composer::updateTrackRect(uint32_t
                                    trackId,
                                    float startXNormalized,
                                    float startYNormalized,
                                    float widthNormalized,
                                    float heightNormalized
                                    ) {
        auto it = mTrackRectMap.find(trackId);
        if (it != mTrackRectMap.end()) {
            auto normalizeChecker = [] (float normalizedValue) -> bool {
                return normalizedValue >= 0 && normalizedValue <= 1;
            };
            auto& rect = (*it).second;
            rect.set(normalizeChecker(startXNormalized)? startXNormalized: rect.startX,
                     normalizeChecker(startYNormalized)? startYNormalized: rect.startY,
                     normalizeChecker(widthNormalized)? widthNormalized: rect.width,
                     normalizeChecker(heightNormalized)? heightNormalized: rect.height);
        }
    }

    uint32_t Composer::getTrackSize() {
        return mTrackList.size();
    }

    std::shared_ptr<RenderTarget> Composer::createRenderTarget(uint32_t trackId) {
        auto surfaceWidth = mWidth;
        auto surfaceHeight = mHeight;
        if (surfaceWidth == 0 || surfaceHeight == 0) return nullptr;
        auto it = mTrackRectMap.find(trackId); //查找获得addTrack时定义的归一化Rect
        if (it != mTrackRectMap.end()) {
            auto rect = (*it).second;
            float _normalizeX = rect.startX;
            float _normalizeY = rect.startY;
            float _normalizeWidth = rect.width;
            float _normalizeHeight = rect.height;
            int x = _normalizeX * surfaceWidth;
            int y = _normalizeY * surfaceHeight;
            int w = _normalizeWidth * surfaceWidth;
            int h = _normalizeHeight * surfaceHeight;
            LOGE("Composer::%s success, surface[%dx%d], track[%d][%d,%d,%d,%d]", __FUNCTION__, surfaceWidth, surfaceHeight, trackId, x, y, w, h);
            return std::make_shared<RenderTarget>(x, y, w, h);
        }
        return nullptr;
    }

    void
    Composer::updateClipMap(const face::TrackEvent &event, const std::shared_ptr <face::Clip> &clip) {
        switch (event) {
            case TrackEvent::AddClip: addComponent(clip); break;
            case TrackEvent::RemoveClip: removeComponent(clip); break;
            default: break;
        }
    }

    void Composer::addComponent(const std::shared_ptr <face::Component> &component) {
        std::lock_guard<std::mutex> lk(mComponentMutex);
        if (!component) return;
        mComponentMap[component->getId()] = component;
        LOGE("Composer::%s success, type:%d, id:%d", __FUNCTION__, component->getType(), component->getId());
    }

    void Composer::removeComponent(const std::shared_ptr <face::Component> &component) {
        std::lock_guard<std::mutex> lk(mComponentMutex);
        if (!component) return;
        mComponentMap.erase(component->getId());
        LOGE("Composer::%s success, type:%d, id:%d", __FUNCTION__, component->getType(), component->getId());
    }

} // face