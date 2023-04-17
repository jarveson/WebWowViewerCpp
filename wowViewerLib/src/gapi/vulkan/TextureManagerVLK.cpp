//
// Created by Deamon on 2/20/2023.
//

#include "GDeviceVulkan.h"
#include "TextureManagerVLK.h"
#include "textures/GBlpTextureVLK.h"

TextureManagerVLK::TextureManagerVLK(IDevice &device) : mdevice(device) {
}

void TextureManagerVLK::initialize() {
    createUpdateCallback();
}

HGTexture TextureManagerVLK::createBlpTexture(HBlpTexture &texture) {
    BlpCacheRecord blpCacheRecord = {
        .texture = std::weak_ptr<HBlpTexture::element_type>(texture)
    };
//    blpCacheRecord.wrapX = xWrapTex;
//    blpCacheRecord.wrapY = yWrapTex;
    std::lock_guard<std::mutex> lock(m_textureAllocation);

    auto i = loadedTextureCache.find(blpCacheRecord);
    if (i != loadedTextureCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            loadedTextureCache.erase(i);
        }
    }

    auto hgTexture = std::make_shared<GBlpTextureVLK>(static_cast<GDeviceVLK&>(mdevice), texture, true, true, onUpdateCallback);

    std::weak_ptr<GBlpTextureVLK> weakPtr(hgTexture);
    {
        std::lock_guard<std::mutex> lock(this->blpTextureLoadMutex);
        m_blpTextLoadQueue.push_back(weakPtr);
    }
    loadedTextureCache[blpCacheRecord] = weakPtr;

    return hgTexture;
}

HGTexture TextureManagerVLK::createTexture() {
    std::shared_ptr<GTextureVLK> h_texture = std::make_shared<GTextureVLK>(static_cast<GDeviceVLK&>(mdevice), true, true, onUpdateCallback);

    return h_texture;
}

void TextureManagerVLK::createUpdateCallback() {
    auto weak_this = this->weak_from_this();
    onUpdateCallback = [weak_this](const std::weak_ptr<GTextureVLK>& texture) -> void {
        if (auto shared_this = weak_this.lock()) {
            std::lock_guard lock(shared_this->textUpdateMutex);

            shared_this->m_texturesReadyToBeUploadedNext.push_back(std::weak_ptr(texture));
        }
    };
}

void TextureManagerVLK::processBLPTextures() {
    std::lock_guard<std::mutex> lock(this->blpTextureLoadMutex);

    auto i = m_blpTextLoadQueue.begin();
    while (i != m_blpTextLoadQueue.end())
    {
        if (auto texture = i->lock()) {
            if (texture->postLoad() == TextureStatus::TSHasUpdates) {
                m_blpTextLoadQueue.erase(i++);
            } else {
                i++;
            }
        } else {
            m_blpTextLoadQueue.erase(i++);
        }
    }
}