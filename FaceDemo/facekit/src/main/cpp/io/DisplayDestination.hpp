//
// Created by wilbert on 2026/1/18.
//

#ifndef FACEDEMO_DISPLAYDESTINATION_HPP
#define FACEDEMO_DISPLAYDESTINATION_HPP
#include "Destination.hpp"
#include "Consumer.hpp"
#include "common/PixelData.hpp"

namespace face {

    class DisplayDestination: public Destination<PixelData>{

    };

} // face

#endif //FACEDEMO_DISPLAYDESTINATION_HPP
