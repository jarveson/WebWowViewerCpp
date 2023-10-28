//
// Created by Deamon on 17.02.23.
//

#ifndef AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H
#define AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H

#include <vector>
#include <string>
#include "../../interface/IDevice.h"
    #include "ISimpleMaterialVLK.h"
#include "../../../renderer/mapScene/vulkan/materials/IMaterialInstance.h"
#include "../descriptorSets/GDescriptorSet.h"

class MaterialBuilderVLK {
public:
    MaterialBuilderVLK(MaterialBuilderVLK const& ) = delete;
    MaterialBuilderVLK& operator=(MaterialBuilderVLK const& ) = delete;

    static MaterialBuilderVLK fromShader(const std::shared_ptr<IDeviceVulkan> &device,
                                         const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig) {
        return {device, shaderFiles, shaderConfig};
    }

    static MaterialBuilderVLK fromMaterial(const std::shared_ptr<IDeviceVulkan> &device,
                                           const std::shared_ptr<ISimpleMaterialVLK> &materialVlk) {
        return {device, materialVlk->getShader(),
                        materialVlk->getPipeline(),
                        materialVlk->getPipeline()->getLayout(),
                        materialVlk->getPipelineTemplate(),
                        toArray(materialVlk->getDescriptorSets()),
                        materialVlk->getMaterialId()
                };
    }
    MaterialBuilderVLK& overridePipelineLayout(const std::unordered_map<int, const std::shared_ptr<GDescriptorSet>> &dses);
    MaterialBuilderVLK& bindDescriptorSet(int bindPoint, std::shared_ptr<GDescriptorSet> &ds);
    MaterialBuilderVLK& createDescriptorSet(int bindPoint, const std::function<void(std::shared_ptr<GDescriptorSet> &ds)> &callback);
    MaterialBuilderVLK& createPipeline(const HGVertexBufferBindings &bindings,
                                       const std::shared_ptr<GRenderPassVLK> &renderPass,
                                       const PipelineTemplate &pipelineTemplate);
    MaterialBuilderVLK& setMaterialId(uint32_t matId);
    std::shared_ptr<ISimpleMaterialVLK> toMaterial();

    template<typename T>
    std::shared_ptr<T> toMaterial(const std::function<void(T *)> &initializer) {
        return std::make_shared<IMaterialInstance<T>>(initializer,
                                   m_shader,
                                   m_pipelineTemplate,
                                   m_pipeline,
                                   m_descriptorSets,
                                   m_materialId);
    }

    ~MaterialBuilderVLK() = default;
private:
    static inline std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> toArray(const std::vector<std::shared_ptr<GDescriptorSet>> &input) {
        std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> res;
        for (int i = 0; i < input.size(); i++) {
            res[i] = input[i];
        }
        return res;
    }
    MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                       const std::vector<std::string> &shaderFiles, const ShaderConfig &shaderConfig);

    MaterialBuilderVLK(const std::shared_ptr<IDeviceVulkan> &device,
                       const std::shared_ptr<GShaderPermutationVLK> &shader,
                       const HPipelineVLK &pipeline,
                       const std::shared_ptr<GPipelineLayoutVLK> &pipelineLayout,
                       const PipelineTemplate &pipelineTemplate,
                       const std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> &descriptorSets,
                       uint32_t materialId);


private:
    //States
    const std::shared_ptr<IDeviceVulkan> &m_device;

    uint32_t m_materialId;
    std::shared_ptr<GShaderPermutationVLK> m_shader;
    HPipelineVLK m_pipeline;
    std::shared_ptr<GPipelineLayoutVLK> m_pipelineLayout;
    PipelineTemplate m_pipelineTemplate;
    std::array<std::shared_ptr<GDescriptorSet>, MAX_SHADER_DESC_SETS> m_descriptorSets;
};


#endif //AWEBWOWVIEWERCPP_MATERIALBUILDERVLK_H
