//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindingsVLK.h"
#include "../../interface/meshes/IMesh.h"
#include "../GDeviceVulkan.h"
#include "../descriptorSets/GDescriptorSet.h"

class GMeshVLK : public IMesh {
    friend class GDeviceVLK;
protected:
    explicit GMeshVLK(IDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMeshVLK() override;

    bool getIsTransparent() override;
    MeshType getMeshType()  override;

public:
    std::shared_ptr<GPipelineVLK> getPipeLineForRenderPass(std::shared_ptr<GRenderPassVLK> renderPass, bool invertedZ);

protected:
    MeshType m_meshType;

    HGShaderPermutation m_shader;

    int8_t m_depthWrite;
    int8_t m_depthCulling;
    int8_t m_backFaceCulling;
    int8_t m_triCCW = 1;

    EGxBlendEnum m_blendMode;
    bool m_isTransparent = false;
    int8_t m_isScissorsEnabled = -1;

    std::array<int, 2> m_scissorOffset = {0,0};
    std::array<int, 2> m_scissorSize = {0,0};

    uint8_t m_colorMask = 0;

    DrawElementMode m_element;


//Vulkan specific
    std::vector<std::shared_ptr<GDescriptorSets>> imageDescriptorSets;
    std::vector<bool> descriptorSetsUpdated;

    std::shared_ptr<GRenderPassVLK> m_lastRenderPass = nullptr;
    bool m_lastInvertedZ = false;
    std::shared_ptr<GPipelineVLK> m_lastPipelineForRenderPass;
private:
    GDeviceVLK &m_device;

    void createDescriptorSets(GShaderPermutationVLK *shaderVLK);
    void updateDescriptor();
};


#endif //WEBWOWVIEWERCPP_GMESH_H
