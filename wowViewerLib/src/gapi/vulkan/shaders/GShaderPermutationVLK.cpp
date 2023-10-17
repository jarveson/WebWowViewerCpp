//
// Created by Deamon on 7/1/2018.
//

#include <iostream>
#include "GShaderPermutationVLK.h"
#include "../../../engine/stringTrim.h"
#include "../../../engine/algorithms/hashString.h"
#include "../../../engine/shader/ShaderDefinitions.h"
#include "../../UniformBufferStructures.h"
#include "../../interface/IDevice.h"
#include <unordered_map>

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule GShaderPermutationVLK::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device->getVkDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    m_device->setObjectName((uint64_t) shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, m_combinedName.c_str());

    return shaderModule;
}

GShaderPermutationVLK::GShaderPermutationVLK(std::string &shaderVertName, std::string &shaderFragName,
                                             const std::shared_ptr<GDeviceVLK> &device,
                                             const ShaderConfig &shaderConf) :
    m_device(device), m_combinedName(shaderVertName + " "+ shaderFragName), m_shaderConf(shaderConf),
    m_shaderNameVert(shaderVertName), m_shaderNameFrag(shaderFragName){
                                             

}

std::vector<const shaderMetaData *> GShaderPermutationVLK::createMetaArray() {
    return {fragShaderMeta, vertShaderMeta};
}

void GShaderPermutationVLK::createSetDescriptorLayouts() {
    std::vector<const shaderMetaData *> metas = createMetaArray();
    for (int i = 0; i < combinedShaderLayout.setLayouts.size(); i++) {
        auto &setLayout = combinedShaderLayout.setLayouts[i];
        if (setLayout.imageBindings.length == 0 && setLayout.uboBindings.length == 0) continue;

        descriptorSetLayouts[i] = std::make_shared<GDescriptorSetLayout>(m_device, metas, i, m_shaderConf.typeOverrides);
    }
}

void GShaderPermutationVLK::compileShader(const std::string &vertExtraDef, const std::string &fragExtraDef) {
    std::string vertShaderName = m_shaderConf.shaderFolder +"/" + m_shaderNameVert;
    std::string vertShaderFrag = m_shaderConf.shaderFolder +"/" + m_shaderNameFrag;

    auto vertShaderCode = readFile("spirv/" + vertShaderName + ".vert.spv");
    auto fragShaderCode = readFile("spirv/" + vertShaderFrag + ".frag.spv");

    vertShaderModule = createShaderModule(vertShaderCode);
    fragShaderModule = createShaderModule(fragShaderCode);

    vertShaderMeta = &shaderMetaInfo.at(vertShaderName + ".vert.spv");
    fragShaderMeta = &shaderMetaInfo.at(vertShaderFrag + ".frag.spv");

    this->createShaderLayout();

    this->createSetDescriptorLayouts();

    this->createPipelineLayout();
}

inline void makeMin(unsigned int &a, const unsigned int b) {
    a = std::min<unsigned int>(a, b);
}
inline void makeMax(unsigned int &a, const unsigned int b) {
    a = std::max<unsigned int>(a, b);
}

void GShaderPermutationVLK::createShaderLayout() {
    //UBO stuff
    for (int i = 0; i < this->vertShaderMeta->uboBindings.size(); i++) {
        auto &uboVertBinding = this->vertShaderMeta->uboBindings[i];

        auto &setLayout = combinedShaderLayout.setLayouts[uboVertBinding.set];

        setLayout.uboSizesPerBinding[uboVertBinding.binding] = uboVertBinding.size;
        makeMin(setLayout.uboBindings.start, uboVertBinding.binding);
        makeMax(setLayout.uboBindings.end, uboVertBinding.binding);
    }
    for (int i = 0; i < this->fragShaderMeta->uboBindings.size(); i++) {
        auto &uboFragBinding = this->fragShaderMeta->uboBindings[i];

        auto &setLayout = combinedShaderLayout.setLayouts[uboFragBinding.set];

        auto it = setLayout.uboSizesPerBinding.find(uboFragBinding.binding);
        if (it != std::end(setLayout.uboSizesPerBinding)) {
            if (it->second != uboFragBinding.size) {
                std::cerr << "sizes mismatch for set = " << uboFragBinding.set
                          << " binding = " << uboFragBinding.binding
                          << " between " << m_shaderNameVert << " and " << m_shaderNameFrag
                          << std::endl;
            }
        } else {
            setLayout.uboSizesPerBinding[uboFragBinding.binding] = uboFragBinding.size;

            makeMin(setLayout.uboBindings.start, uboFragBinding.binding);
            makeMax(setLayout.uboBindings.end, uboFragBinding.binding);
        }
    }

    //Image stuff
    for (int i = 0; i < this->vertShaderMeta->imageBindings.size(); i++) {
        auto &imageVertBinding = this->vertShaderMeta->imageBindings[i];
        auto &setLayout = combinedShaderLayout.setLayouts[imageVertBinding.set];

        if (setLayout.uboSizesPerBinding.find(imageVertBinding.binding) != std::end(setLayout.uboSizesPerBinding)) {
            std::cerr << "types mismatch. image slot is used for UBO. for set = " << imageVertBinding.set
                      << " binding = " << imageVertBinding.binding
                      << " in " << m_shaderNameVert << " and " << m_shaderNameFrag
                      << std::endl;
            throw std::runtime_error("types mismatch");
        }

        makeMin(setLayout.imageBindings.start, imageVertBinding.binding);
        makeMax(setLayout.imageBindings.end, imageVertBinding.binding);
    }
    for (int i = 0; i < this->fragShaderMeta->imageBindings.size(); i++) {
        auto &imageFragBinding = this->fragShaderMeta->imageBindings[i];

        auto &setLayout = combinedShaderLayout.setLayouts[imageFragBinding.set];

        if (setLayout.uboSizesPerBinding.find(imageFragBinding.binding) != std::end(setLayout.uboSizesPerBinding)) {
            std::cerr << "types mismatch. image slot is used for UBO. for set = " << imageFragBinding.set
                      << " binding = " << imageFragBinding.binding
                      << " in " << m_shaderNameVert << " and " << m_shaderNameFrag
                      << std::endl;
            throw std::runtime_error("types mismatch");
        }

        makeMin(setLayout.imageBindings.start, imageFragBinding.binding);
        makeMax(setLayout.imageBindings.end, imageFragBinding.binding);
    }
    //Cleanup
    for (auto &shaderLayout : combinedShaderLayout.setLayouts) {
        {
            auto &data = shaderLayout.uboBindings;
            if (shaderLayout.uboBindings.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }
        {
            auto &data = shaderLayout.imageBindings;
            if (shaderLayout.uboBindings.start < 100) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }
    }
}
std::shared_ptr<GPipelineLayoutVLK>
GShaderPermutationVLK::createPipelineLayoutOverrided(const std::unordered_map<int, const std::shared_ptr<GDescriptorSet>> &dses) {
    std::vector<VkDescriptorSetLayout> descLayouts;
    descLayouts.reserve(combinedShaderLayout.setLayouts.size());
    for (int i = 0; i < combinedShaderLayout.setLayouts.size(); i++) {

        if (dses.find(i) != dses.end()) {
            descLayouts.push_back(dses.at(i)->getDescSetLayout()->getSetLayout());
        } else {
            descLayouts.push_back(this->getDescriptorLayout(i)->getSetLayout());
        }
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    pipelineLayoutInfo.setLayoutCount = descLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descLayouts.data();

    std::cout << "Pipeline layout for "+this->getShaderCombinedName() << std::endl;

    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(m_device->getVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    auto combinedName = this->getShaderCombinedName();
    m_device->setObjectName(reinterpret_cast<uint64_t>(pipelineLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT, combinedName.c_str());

    return std::make_shared<GPipelineLayoutVLK>(*m_device, pipelineLayout);
}

void GShaderPermutationVLK::createPipelineLayout() {
    m_pipelineLayout = createPipelineLayoutOverrided({});
}

const std::shared_ptr<GDescriptorSetLayout>
GShaderPermutationVLK::getDescriptorLayout(int bindPoint) {
    return descriptorSetLayouts[bindPoint];
}

