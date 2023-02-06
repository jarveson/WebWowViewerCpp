//
// Created by Deamon on 03.02.23.
//

#include <algorithm>
#include "GDescriptorSetLayout.h"

GDescriptorSetLayout::GDescriptorSetLayout(std::shared_ptr<GDeviceVLK> &device, const std::vector<const shaderMetaData*> &metaDatas, int setIndex) : m_device(device) {
    //Create Layout
    std::unordered_map<int,VkDescriptorSetLayoutBinding> shaderLayoutBindings;

    for (const auto p_metaData : metaDatas) {
        auto const &metaData = *p_metaData;

        VkShaderStageFlagBits vkStageFlag = [](ShaderStage stage) -> VkShaderStageFlagBits {
            switch (stage) {
                case ShaderStage::Vertex:           return VK_SHADER_STAGE_VERTEX_BIT; break;
                case ShaderStage::Fragment:         return VK_SHADER_STAGE_FRAGMENT_BIT; break;
                case ShaderStage::RayGenerate:      return VK_SHADER_STAGE_RAYGEN_BIT_KHR; break;
                case ShaderStage::RayAnyHit:        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; break;
                case ShaderStage::RayClosestHit:    return VK_SHADER_STAGE_ANY_HIT_BIT_KHR; break;
                case ShaderStage::RayMiss:          return VK_SHADER_STAGE_MISS_BIT_KHR; break;
                default:
                    return (VkShaderStageFlagBits)0;
            }
        }(metaData.stage);



        for (int i = 0; i < p_metaData->uboBindings.size(); i++) {
            auto &uboBinding = p_metaData->uboBindings[i];

            if (uboBinding.set != setIndex) return;

            auto it = shaderLayoutBindings.find(uboBinding.binding);
            if (it != std::end( shaderLayoutBindings )) {
                it->second.stageFlags |= vkStageFlag;
                if (it->second.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                    std::cerr << "Type mismatch for ubo in GDescriptorSetLayout" << std::endl;
                    throw std::runtime_error("types mismatch");
                }
            } else {
                VkDescriptorSetLayoutBinding uboLayoutBinding = {};
                uboLayoutBinding.binding = uboBinding.binding;
                uboLayoutBinding.descriptorCount = 1;
                uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uboLayoutBinding.pImmutableSamplers = nullptr;
                uboLayoutBinding.stageFlags = vkStageFlag;

                shaderLayoutBindings.insert({uboBinding.binding, uboLayoutBinding});
                m_totalUbos++;
            }
        }

        for (int i = 0; i < p_metaData->imageBindings.size(); i++) {
            auto &imageBinding = p_metaData->imageBindings[i];

            if (imageBinding.set != setIndex) return;

            auto it = shaderLayoutBindings.find(imageBinding.binding);
            if (it != std::end( shaderLayoutBindings )) {
                it->second.stageFlags |= vkStageFlag;
                if (it->second.descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    std::cerr << "Type mismatch for image in GDescriptorSetLayout" << std::endl;
                    throw std::runtime_error("types mismatch");
                }
            } else {
                VkDescriptorSetLayoutBinding imageLayoutBinding = {};
                imageLayoutBinding.binding = p_metaData->imageBindings[i].binding;
                imageLayoutBinding.descriptorCount = 1;
                imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                imageLayoutBinding.pImmutableSamplers = nullptr;
                imageLayoutBinding.stageFlags = vkStageFlag;

                shaderLayoutBindings.insert({imageBinding.binding, imageLayoutBinding});
                m_totalImages++;
            }
        }
    }

    std::vector<VkDescriptorSetLayoutBinding> layouts(shaderLayoutBindings.size());
    std::transform(shaderLayoutBindings.begin(), shaderLayoutBindings.end(), layouts.begin(), [](auto &pair){return pair.second;});


    //Create VK descriptor layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = shaderLayoutBindings.size();
    layoutInfo.pBindings = (shaderLayoutBindings.size() > 0) ? &shaderLayoutBindings[0] : nullptr;

    if (vkCreateDescriptorSetLayout(m_device->getVkDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

GDescriptorSetLayout::~GDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(m_device->getVkDevice(), m_descriptorSetLayout, nullptr);
}