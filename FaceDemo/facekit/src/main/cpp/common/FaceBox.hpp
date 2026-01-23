//
// Created by wilbert on 2026/1/23.
//

#ifndef FACEDEMO_FACEBOX_HPP
#define FACEDEMO_FACEBOX_HPP

namespace face{
    struct Box
    {
        /*
         * cx: x of box center
         * cy: y of box center
         * sx: width of box
         * sy: height of box
         */

        float cx;
        float cy;
        float sx;
        float sy;
    };

    struct Point {
        float x;
        float y;
    };

    struct BBox
    {
        float x1;
        float y1;
        float x2;
        float y2;
        float score;
        Point landmarks[5];
    };
}
#endif //FACEDEMO_FACEBOX_HPP
