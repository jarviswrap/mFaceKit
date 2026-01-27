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
        VideoTrack(): Track(TrackType::Video) {};
        void resize(uint32_t width, uint32_t height);

        std::shared_ptr<Texture> getCurrentTexture() { return mCurrentTexture; };

        void getSize(uint32_t& width, uint32_t& height) const { mTrackSize.getSize(width, height); };
    protected:
        void onRender(std::shared_ptr<Clip> clip, uint64_t timeStamp) override;
        void onThreadStart() override;
        void onThreadStop() override;

    private:
        std::shared_ptr<Texture> mCurrentTexture;
        Size<uint32_t> mTrackSize{0, 0};
        std::shared_ptr<EGLEnvironment> mGLEnvironment;
        std::vector<std::shared_ptr<FrameBuffer>> mFrameBufferList;
        uint32_t mFrameBufferId{0};
        bool mSizeChanged{true};
        static constexpr int FRAME_BUFFER_SIZE = 2;
        bool initRenderEnv();
    };

} // face

#endif //FACEDEMO_VIDEOTRACK_HPP
