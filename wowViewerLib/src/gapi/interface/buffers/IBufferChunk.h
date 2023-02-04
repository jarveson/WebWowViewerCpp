    //
// Created by deamon on 09.12.19.
//

#ifndef AWEBWOWVIEWERCPP_IBUFFERCHUNK_H
#define AWEBWOWVIEWERCPP_IBUFFERCHUNK_H

#include <functional>
#include <assert.h>
#include "IBuffer.h"
#include "../../../renderer/mapScene/FrameDependentData.h"

template <typename T>
using IChunkHandlerType = std::function<void(T &data, const HFrameDependantData &frameDepedantData)> ;

template<typename T>
class IBufferChunk {
public:
    virtual ~IBufferChunk() = default;
    virtual T &getObject() = 0;
    virtual void save() = 0;

    virtual void setUpdateHandler(IChunkHandlerType<T> handler) {};
};
#endif //AWEBWOWVIEWERCPP_IBUFFERCHUNK_H
