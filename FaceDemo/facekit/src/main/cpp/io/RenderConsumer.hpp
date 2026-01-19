//
// Created by wilbert on 2026/1/18.
//

#ifndef FACEDEMO_RENDERCONSUMER_HPP
#define FACEDEMO_RENDERCONSUMER_HPP
#include "Consumer.hpp"
#include "common/PixelData.hpp"

namespace face {

    class RenderConsumer: public Consumer<PixelData> {
    public:
        RenderConsumer();
        ~RenderConsumer();

        Error onRequestConsume(uint32_t requestId) override;
        Error onConsumeData(PixelData data) override;
    };

} // face

#endif //FACEDEMO_RENDERCONSUMER_HPP
