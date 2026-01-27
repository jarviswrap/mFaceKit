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

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;
struct SwsContext;

namespace face {

    class VideoClip: public Clip, public std::enable_shared_from_this<VideoClip> {
    public:
        explicit VideoClip(const std::string& videoFile);
        ~VideoClip();
        bool render(uint64_t timeStamp) override;
        void onSizeChanged(uint32_t width, uint32_t height);
    private:
        uint32_t mWidth{0};
        uint32_t mHeight{0};
        std::shared_ptr<PixelRender> mPixelRender{nullptr};
        std::shared_ptr<LimitQueue<RenderData<PixelData>>> mDataQueue{nullptr};
        std::shared_ptr<LimitQueue<RenderData<PixelData>>> mRecycleQueue{nullptr};
        std::shared_ptr<LoopThread> mThread{nullptr};
        std::string mVideoFile{""};
        
        AVFormatContext* mFormatCtx{nullptr};
        AVCodecContext* mCodecCtx{nullptr};
        const AVCodec* mCodec{nullptr};
        AVFrame* mFrame{nullptr};
        AVPacket* mPacket{nullptr};
        SwsContext* mSwsCtx{nullptr};
        int mVideoStreamIndex{-1};

        void onStart();
        void onLoop(uint64_t requestId);
        void onStop();
        std::shared_ptr<RenderData<PixelData>> getEmptyData();
    };

} // face

#endif //FACEDEMO_VIDEOCLIP_HPP
