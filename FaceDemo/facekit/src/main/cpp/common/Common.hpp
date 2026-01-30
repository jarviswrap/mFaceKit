//
// Created by wilbert on 2026/1/15.
//

#ifndef FACEDEMO_COMMON_HPP
#define FACEDEMO_COMMON_HPP

#include <functional>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p){delete p; p = NULL;}
#endif

namespace face {
    template<typename... T> using DataListener = std::function<void(const T &...)>;
    template<typename... T> using BoolListener = std::function<bool(const T &...)>;
    template<typename... T> using IntListener = std::function<int(const T &...)>;
    template<typename T> using DataConverter = std::function<T(const T &)>;

    template<typename T> using DataObtainer = std::function<T(uint32_t requestCode)>;

    enum class Error: int8_t {
        None                   = 0,
        Err_ModelInvalid       = -1,
        Err_InvalidSource      = -2,
        Err_InvalidInterpreter = -3,
        Err_EmptyQueue         = -4,
        Err_InvalidInput       = -5,
        Err_InvalidData        = -6,
        Err_InvalidJniMethod   = -7,
        Err_InvalidSurface     = -8,
        Err_InvalidConsumer    = -9,
        Err_InvalidShowView    = -10,
        Err_StartTwice         = -11,
        Err_InvalidProgram     = -12,
        Err_OpenGLError        = -13,
    };
}
#endif //FACEDEMO_COMMON_HPP
