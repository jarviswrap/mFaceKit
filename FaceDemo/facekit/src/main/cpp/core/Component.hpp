//
// Created by wilbert on 2026/1/29.
//

#ifndef FACEDEMO_COMPONENT_HPP
#define FACEDEMO_COMPONENT_HPP
#include <string>
#include <memory>
#include "common/Log.hpp"

namespace face {
    enum class ComponentType{
        Unknown, Track, Clip,
    };

    class ComponentId {
    public:
        ComponentId(const std::string& name, ComponentType type): mName(name), mType(type) {
            if (mType == ComponentType::Track) {
                static uint32_t trackIndex = 0;
                trackIndex = trackIndex + 1;
                mId = trackIndex;
                LOGE("ComponentId Track:%lu", mId);
            } else if (mType == ComponentType::Clip) {
                static uint32_t clipIndex = 1000; // 0～1000预留给trackIndex
                clipIndex = clipIndex + 1;
                mId = clipIndex;
                LOGE("ComponentId Clip:%lu", mId);
            }
        }

        bool isValid() const { return mId > 0; }
        uint32_t getId() const { return mId; }
        std::string getName() const { return mName; }
        ComponentType getType() const { return mType; }
    private:
        uint32_t mId{0};
        std::string mName{""};
        ComponentType mType{ComponentType::Unknown};
    };

    class Component{
    public:
        Component(ComponentType type, const std::string& name):mComponentId(std::make_shared<ComponentId>(name, type)) {};
        virtual ~Component() = default;

        ComponentType getType() const { return mComponentId->getType(); }
        std::shared_ptr<ComponentId> getComponentId() const { return mComponentId; }
        uint32_t getId() const { return mComponentId->getId(); }    
        std::string getName() const { return mComponentId->getName(); }
    protected:
        std::shared_ptr<ComponentId> mComponentId{nullptr};
    };

} // face

#endif //FACEDEMO_COMPONENT_HPP
