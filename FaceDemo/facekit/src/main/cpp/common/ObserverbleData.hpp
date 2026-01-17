//
// Created by wilbert on 2026/1/17.
//

#ifndef FACEDEMO_OBSERVERBLEDATA_HPP
#define FACEDEMO_OBSERVERBLEDATA_HPP
#include "common/Common.hpp"

namespace face {

    template<typename T>
    class ObservableData {
    public:
        explicit ObservableData(DataListener<T> observer = nullptr) { setDataObserver(observer); }
        
        void setDataObserver(DataListener<T> observer) {
            mObserver = observer;
        }
        
        /**
         * 设置数据，仅当数据改变或者forceNotify为true时触发观察者
         * 要求T实现了operator==
         * forceNotify: 强制设置数据并触发观察者，无论数据是否改变
         */
        void setData(const T& data, bool forceNotify = false) {
            if (data != mData || forceNotify) {
                mData = data;
                if (mObserver) {
                    mObserver(data);
                }
            }
        }
        
        /**
         * 获取当前数据
         */
        const T& getData() const {
            return mData;
        }
        
    private:
        T mData{};
        DataListener<T> mObserver;
    };

} // face

#endif //FACEDEMO_OBSERVERBLEDATA_HPP
