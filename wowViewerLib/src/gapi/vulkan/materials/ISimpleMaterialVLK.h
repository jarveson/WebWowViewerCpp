//
// Created by Deamon on 29.12.22.
//

#ifndef AWEBWOWVIEWERCPP_ISIMPLEMATERIALVLK_H
#define AWEBWOWVIEWERCPP_ISIMPLEMATERIALVLK_H


#include <vector>
#include <memory>
#include "../../interface/materials/IMaterial.h"
#include "../descriptorSets/GDescriptorSet.h"
#include "../textures/GTextureVLK.h"

class ISimpleMaterialVLK : public IMaterial {
public:
    explicit ISimpleMaterialVLK(const HGDeviceVLK &device, const std::string &vertexShader, const std::string &pixelShader,

                                const std::vector<std::shared_ptr<IBufferVLK>> &ubos,
                                const std::vector<std::shared_ptr<GTextureVLK>> &textures);
    ~ISimpleMaterialVLK() override = default;

    HGShaderPermutation getShader() {
        return m_shader;
    }

    void createImageDescriptorSet();
    void updateImageDescriptorSet();

private:
    HGDeviceVLK m_device;

    std::vector<HGTextureVLK> m_textures = {};
    std::vector<std::shared_ptr<IBufferVLK>> m_ubos = {};

    std::vector<std::shared_ptr<GDescriptorSet>> imageDescriptorSets;
    std::vector<std::shared_ptr<GDescriptorSet>> uboDescriptorSets;
    std::vector<std::shared_ptr<GDescriptorSet>> ssboDescriptorSets;

    HGShaderPermutation m_shader;
};

typedef std::shared_ptr<ISimpleMaterialVLK> HMaterialVLK;

#endif //AWEBWOWVIEWERCPP_ISIMPLEMATERIALVLK_H
