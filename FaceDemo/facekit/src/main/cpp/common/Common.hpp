//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_COMMON_HPP
#define FACEDEMO_COMMON_HPP

#include <functional>

namespace face
{
    template<typename... T> using DataListener = std::function<void(const T &...)>;
    template<typename... T> using BoolListener = std::function<bool(const T &...)>;
    template<typename... T> using IntListener = std::function<int(const T &...)>;

    template<typename T> using DataObtainer = std::function<T(uint32_t requestCode)>;

    enum class Error : int8_t
    {
        None             = 0,
        Err_ModelInvalid = -1,
        Err_InvalidSource = -2,
        Err_InvalidInterpreter = -3,
        Err_EmptyQueue = -4,
        Err_InvalidInput = -5,
    };
}
#endif //FACEDEMO_COMMON_HPP
