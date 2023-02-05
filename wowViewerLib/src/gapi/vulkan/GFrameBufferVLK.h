//
// Created by Deamon on 12/11/2020.
//

#ifndef AWEBWOWVIEWERCPP_GFRAMEBUFFERVLK_H
#define AWEBWOWVIEWERCPP_GFRAMEBUFFERVLK_H

#include <vector>
#include "../interface/IFrameBuffer.h"
#include "../interface/textures/ITexture.h"
#include "GDeviceVulkan.h"

class GFrameBufferVLK : public IFrameBuffer {
public:
    GFrameBufferVLK(IDevice &device, const std::vector<ITextureFormat> &textureAttachments, ITextureFormat depthAttachment, int multiSampleCnt, int width, int height);
    ~GFrameBufferVLK() override;

    void readRGBAPixels(int x, int y, int width, int height, void *data) override;
    HGTexture getAttachment(int index) override;
    HGTexture getDepthTexture() override;
    void bindFrameBuffer() override;
    void copyRenderBufferToTexture() override;

    static void iterateOverAttachments(const std::vector<ITextureFormat> &textureAttachments, std::function<void(int i, VkFormat textureFormat)> callback);

    VkFramebuffer getFrameBuffer() {return m_frameBuffer;};
    int getColorOrDataAttachmentCount() { return attachmentFormats.size(); };

private:
    GDeviceVLK &mdevice;

    std::shared_ptr<GRenderPassVLK> m_renderPass;
    VkFramebuffer m_frameBuffer;

    std::vector<HGTexture> attachmentTextures;
    HGTexture depthTexture;

    //Used only in readRGBAPixels function
    std::vector<VkFormat> attachmentFormats;

    int m_multiSampleCnt;

    int m_width = 0;
    int m_height = 0;
};



#endif //AWEBWOWVIEWERCPP_GFRAMEBUFFERVLK_H
