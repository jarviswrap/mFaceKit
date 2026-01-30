//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_VIDEOTRACK_HPP
#define FACEDEMO_VIDEOTRACK_HPP
#include "Track.hpp"
#include "io/Source.hpp"
#include "io/Destination.hpp"
#include "common/PixelData.hpp"
#include "render/Texture.hpp"
#include <atomic>
namespace face {

    class FrameBuffer;
    class EGLEnvironment;
    class VideoTrack: public Track{
    public:
        VideoTrack();
        void setRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget);
        const std::shared_ptr<RenderTarget>& getRenderTarget() const { return mRenderTarget; }
        std::shared_ptr<FrameBuffer> getCurrentFrameBuffer();
        void recycleFrameBuffer(std::shared_ptr<FrameBuffer>&& frameBuffer);

        std::atomic<bool> isUsing{false};
    protected:
        void onRender(std::shared_ptr<Clip> clip, uint64_t timeStamp) override;
        void onThreadStart() override;
        void onThreadStop() override;

    private:
        std::shared_ptr<RenderTarget> mRenderTarget{nullptr};
        std::shared_ptr<LimitQueue<FrameBuffer>> mDataQueue{nullptr};
        std::shared_ptr<LimitQueue<FrameBuffer>> mRecycleQueue{nullptr};

        std::shared_ptr<Texture> mCurrentTexture{nullptr};
        std::shared_ptr<EGLEnvironment> mGLEnvironment{nullptr};
        std::vector<std::shared_ptr<FrameBuffer>> mFrameBufferList;
        std::shared_ptr<FrameBuffer> mCurrentFrameBuffer{nullptr};

        static constexpr int FRAME_BUFFER_SIZE = 2;
        bool initRenderEnv();
    };

} // face

#endif //FACEDEMO_VIDEOTRACK_HPP
