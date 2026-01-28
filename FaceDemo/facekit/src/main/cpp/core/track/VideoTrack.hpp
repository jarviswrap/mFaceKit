//
// Created by wilbert on 2026/1/26.
//

#ifndef FACEDEMO_VIDEOTRACK_HPP
#define FACEDEMO_VIDEOTRACK_HPP
#include "Track.hpp"
#include "io/Source.hpp"
#include "io/Destination.hpp"
#include "common/PixelData.hpp"
#include "render/utils/Texture.hpp"
#include <atomic>
namespace face {

    class FrameBuffer;
    class EGLEnvironment;
    class VideoTrack: public Track{
    public:
        VideoTrack();
        void resize(uint32_t width, uint32_t height);

        std::shared_ptr<FrameBuffer> getCurrentFrameBuffer();
        void recycleFrameBuffer(std::shared_ptr<FrameBuffer>&& frameBuffer);

        void getSize(uint32_t& width, uint32_t& height) const { mTrackSize.getSize(width, height); };

        std::atomic<bool> isUsing{false};
    protected:
        void onRender(std::shared_ptr<Clip> clip, uint64_t timeStamp) override;
        void onThreadStart() override;
        void onThreadStop() override;

    private:
        std::shared_ptr<LimitQueue<FrameBuffer>> mDataQueue{nullptr};
        std::shared_ptr<LimitQueue<FrameBuffer>> mRecycleQueue{nullptr};

        std::shared_ptr<Texture> mCurrentTexture{nullptr};
        Size<uint32_t> mTrackSize{0, 0};
        std::shared_ptr<EGLEnvironment> mGLEnvironment{nullptr};
        std::vector<std::shared_ptr<FrameBuffer>> mFrameBufferList;
        std::shared_ptr<FrameBuffer> mCurrentFrameBuffer{nullptr};
        uint32_t mFrameBufferId{0};
        std::atomic<bool> mSizeChanged{true};

        static constexpr int FRAME_BUFFER_SIZE = 2;
        bool initRenderEnv();
        std::shared_ptr<FrameBuffer> getEmptyFrameBuffer();

    };

} // face

#endif //FACEDEMO_VIDEOTRACK_HPP
