//
// Created by Deamon on 12.12.22.
//

#include "FrontendUIRenderForwardVLK.h"
#include "../../../../../wowViewerLib/src/gapi/UniformBufferStructures.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/materials/ISimpleMaterialVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/buffers/IBufferChunkVLK.h"
#include "../../../../../wowViewerLib/src/gapi/vulkan/meshes/GMeshVLK.h"

FrontendUIRenderForwardVLK::FrontendUIRenderForwardVLK(const HGDeviceVLK &hDevice) : FrontendUIRenderer(
    hDevice), m_device(hDevice) {
}

void FrontendUIRenderForwardVLK::createBuffers() {
    vboBuffer = m_device->createIndexBuffer(1024*1024);
    iboBuffer = m_device->createVertexBuffer(1024*1024);
    uboBuffer = m_device->createUniformBuffer(sizeof(ImgUI::modelWideBlockVS)*IDevice::MAX_FRAMES_IN_FLIGHT);

    m_imguiUbo = std::make_shared<CBufferChunkVLK<ImgUI::modelWideBlockVS>>(uboBuffer);
}

HGVertexBuffer FrontendUIRenderForwardVLK::createVertexBuffer(int sizeInBytes) {
    return vboBuffer->getSubBuffer(sizeInBytes);
}

HGIndexBuffer FrontendUIRenderForwardVLK::createIndexBuffer(int sizeInBytes) {
    return iboBuffer->getSubBuffer(sizeInBytes);
}

HMaterial FrontendUIRenderForwardVLK::createUIMaterial(const UIMaterialTemplate &materialTemplate) {
    auto i = m_materialCache.find(materialTemplate);
    if (i != m_materialCache.end()) {
        if (!i->second.expired()) {
            return i->second.lock();
        } else {
            m_materialCache.erase(i);
        }
    }

    std::vector<std::shared_ptr<IBufferVLK>> ubos = {};
    std::vector<HGTextureVLK> texturesVLK = {std::dynamic_pointer_cast<GTextureVLK>(materialTemplate.texture)};
    auto material = std::make_shared<ISimpleMaterialVLK>(m_device,
                                  "imguiShader", "imguiShader",
                                  ubos,
                                  texturesVLK);

    std::weak_ptr<ISimpleMaterialVLK> weakPtr(material);
    m_materialCache[materialTemplate] = weakPtr;

    return material;
}

HGMesh FrontendUIRenderForwardVLK::createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) {
    //TODO:
    return std::make_shared<GMeshVLK>(*m_device, meshTemplate, std::dynamic_pointer_cast<ISimpleMaterialVLK>(material));
}

void FrontendUIRenderForwardVLK::updateAndDraw(
    const std::shared_ptr<FrameInputParams<ImGuiFramePlan::ImGUIParam>> &frameInputParams,
    const std::shared_ptr<ImGuiFramePlan::EmptyPlan> &framePlan) {

    std::vector<HGMesh> meshes;
    this->consumeFrameInput(frameInputParams, meshes);

    //Record commands to update buffer and draw
    [&meshes](VkCommandBuffer transferQueueCMD, VkCommandBuffer renderFB, VkCommandBuffer renderSwapFB) {

    };
}