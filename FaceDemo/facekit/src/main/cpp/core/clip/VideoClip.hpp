//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_VIDEOCLIP_HPP
#define FACEDEMO_VIDEOCLIP_HPP
#include "Clip.hpp"
#include <string>
#include "common/LoopThread.h"
#include "common/LimitQueue.hpp"
#include "common/PixelData.hpp"
#include "render/PixelRender.hpp"
#include "render/InspectionRender.hpp"

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;
struct SwsContext;

namespace face {

    class VideoClip: public Clip, public std::enable_shared_from_this<VideoClip> {
    public:
        explicit VideoClip(const std::string& videoFile, std::shared_ptr<ComponentId> trackId);
        ~VideoClip();
        void start(uint64_t start, uint64_t end);
        bool render(uint64_t timeStamp, const std::shared_ptr<RenderTarget>& renderTarget) override;
        void addRender(const std::shared_ptr<Render<Texture>>& render, const std::shared_ptr<Inspector<PixelData>>& inspector = nullptr);

        bool releaseRender() override;
    private:
        std::mutex mMutex;
        std::vector<std::shared_ptr<Inspector<PixelData>>> mInspectors;
        std::vector<std::shared_ptr<Render<Texture>>> mEffectRenders; // 特效渲染器，最后一个渲染器绘制到输入的renderTarget上
        std::shared_ptr<PixelRender> mPixelRender{nullptr};
        std::shared_ptr<Render<Texture>> mOutputRender{nullptr}; // 最后一个渲染器

        std::shared_ptr<LimitQueue<PixelData>> mDataQueue{nullptr};
        std::shared_ptr<LimitQueue<PixelData>> mRecycleQueue{nullptr};
        std::shared_ptr<LimitQueue<FrameBuffer>> mFrameBuffers{nullptr};

        std::shared_ptr<LoopThread> mThread{nullptr};
        std::string mVideoFile{""};
        
        AVFormatContext* mFormatCtx{nullptr};
        AVCodecContext* mCodecCtx{nullptr};
        const AVCodec* mCodec{nullptr};
        AVFrame* mFrame{nullptr};
        AVPacket* mPacket{nullptr};
        SwsContext* mSwsCtx{nullptr};
        int mVideoStreamIndex{-1};

        void seek(uint64_t timeStamp);
        void flush();

        void onStart();
        void onLoop(uint64_t requestId);
        void onStop();
        void onInspect(const std::shared_ptr<PixelData>& data);
        std::shared_ptr<PixelData> getEmptyData();
        std::shared_ptr<PixelData> getData(uint64_t timeStamp);
        std::shared_ptr<FrameBuffer> getCachedTarget(const std::shared_ptr<RenderTarget>& renderTarget);
    };

} // face

#endif //FACEDEMO_VIDEOCLIP_HPP
