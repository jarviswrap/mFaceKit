//
// Created by wilbert on 2026/1/16.
//

#ifndef FACEDEMO_IMAGESOURCE_HPP
#define FACEDEMO_IMAGESOURCE_HPP
#include "Source.hpp"

namespace face
{

    class ImageSource: Source
    {

    public:
        ImageSource(const std::string& imagePath);
        ~ImageSource() override;
    };

} // face

#endif //FACEDEMO_IMAGESOURCE_HPP
