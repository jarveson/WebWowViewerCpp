//
// Created by deamon on 04.12.19.
//

#ifndef AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H
#define AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H

#include <string>
#include <vector>
#include <iostream>
#include "fileHelpers.h"
#include "webGLSLCompiler.h"

struct attributeDefine {
    std::string name;
    unsigned int location;
};

struct fieldDefine {
    std::string name;
    bool isFloat ;
    int offset;
    int columns;
    int vecSize;
    int arraySize;
};

struct uboBindingData {
    unsigned int set;
    unsigned int binding;
    unsigned long long size;
};

struct imageBindingData {
    unsigned int set;
    unsigned int binding;
    std::string imageName;
};

struct imageBindingAmountData {
    unsigned int start = 999;
    unsigned int end = 0;
    unsigned int length = 0;

};

struct shaderMetaData {
    std::vector<uboBindingData> uboBindings;
    std::vector<imageBindingData> imageBindings;
    std::array<imageBindingAmountData, 8> imageBindingAmounts;
};

//Per file
std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;


void dumpMembers(spirv_cross::WebGLSLCompiler &glsl, std::vector<fieldDefine> &fieldDefines, spirv_cross::TypeID type, std::string namePrefix, int offset, int currmemberSize) {
    auto memberType = glsl.get_type(type);
    int arraySize = memberType.array.size() > 0 ? memberType.array[0] : 0;
    bool arrayLiteral = memberType.array_size_literal.size() > 0 ? memberType.array_size_literal[0] : 0;
    int vecSize = memberType.vecsize;
    int width = memberType.width;
    int columns = memberType.columns;


    bool isStruct = memberType.basetype == spirv_cross::SPIRType::Struct;

    if (isStruct) {
        auto parentTypeId = memberType.parent_type;
        if (parentTypeId == spirv_cross::TypeID(0)) {
            parentTypeId = memberType.type_alias;
        }
        if (parentTypeId == spirv_cross::TypeID(0)) {
            parentTypeId = memberType.self;
        }
//
//        auto submemberType = glsl.get_type(submemberTypeId);
//        int structSize = submemberType.vecsize * submemberType.columns*(submemberType.width/8);

        if (arrayLiteral) {
            for (int j = 0; j < arraySize; j++) {
                for (int k = 0; k < memberType.member_types.size(); k++) {
                    auto memberName = glsl.get_member_name(parentTypeId, k);
                    auto memberOffset = glsl.type_struct_member_offset(memberType, k);
                    auto memberSize = glsl.get_declared_struct_member_size(memberType, k);

                    dumpMembers(glsl, fieldDefines, memberType.member_types[k],
                                namePrefix + "[" + std::to_string(j) + "]" + "." + memberName, offset+(j*(currmemberSize/arraySize))+memberOffset, memberSize);
                }
            }
        } else {
            for (int k = 0; k < memberType.member_types.size(); k++) {
                auto memberName = glsl.get_member_name(parentTypeId, k);
                auto memberSize = glsl.get_declared_struct_member_size(memberType, k);
                auto memberOffset = glsl.type_struct_member_offset(memberType, k);
                dumpMembers(glsl, fieldDefines, memberType.member_types[k], namePrefix + "_" + memberName, offset+memberOffset, memberSize);
            }
        }
    } else {
        bool isFloat = (memberType.basetype == spirv_cross::SPIRType::Float);
//        std::cout << "{\"" <<namePrefix <<"\","<< isFloat << ", " <<columns<<","<<vecSize<<","<<arraySize << "}"<<std::endl;
        fieldDefines.push_back({namePrefix, isFloat, offset, columns,vecSize,arraySize});

//        std::cout <<
//                  namePrefix <<
//                  ", columns = " << columns <<
//                  ", isFloat = " << (memberType.basetype == spirv_cross::SPIRType::Float) <<
//                  ", memberSize = " << currmemberSize <<
//                  ", vecSize = " << vecSize <<
//                  ", width = " << width <<
//                  ", arraySize = " << arraySize <<
//                  ", arrayLiteral = " << arrayLiteral <<
//                  "  offset = " << offset << std::endl;
    }
}

void dumpShaderUniformOffsets(std::vector<std::string> &shaderFilePaths) {
    std::cout << "#ifndef WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H\n"
                 "#define WOWMAPVIEWERREVIVED_SHADERDEFINITIONS_H\n"
                 "\n"
                 "#include <string>\n"
                 "#include <iostream>\n"
                 "#include <fstream>\n"
                 "#include <vector>\n"
                 "#include <unordered_map>\n"
                 "\n"
                 "template <typename T>\n"
                 "inline constexpr const uint32_t operator+ (T const val) { return static_cast<const uint32_t>(val); };"
              << std::endl;


    std::cout << "struct fieldDefine {\n"
                 "    std::string name;\n"
                 "    bool isFloat ;\n"
                 "    int offset;\n"
                 "    int columns;\n"
                 "    int vecSize;\n"
                 "    int arraySize;\n"
                 "};" << std::endl;

    std::cout << "struct attributeDefine {\n"
                 "    std::string name;\n"
                 "    int location;\n"
                 "};" << std::endl;

    std::cout <<
    R"===(
    struct uboBindingData {
        int set;
        int binding;
        int size;
    };
    struct imageBindingData {
        unsigned int set;
        unsigned int binding;
        std::string imageName;
    };

    struct imageBindingAmountData {
        unsigned int start = 999;
        unsigned int end = 0;
        unsigned int length = 0;

    };

    struct shaderMetaData {
        std::vector<uboBindingData> uboBindings;
        std::vector<imageBindingData> imageBindings;
        std::array<imageBindingAmountData, 8> imageBindingAmounts;
    };
    //Per file
    extern const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo;
    extern const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;

    extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;

    extern const std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;
)===" << std::endl;


    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert;
    std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag;
    std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName;

    //1. Collect data
    for (auto &filePath : shaderFilePaths) {

//        std::cerr << filePath << std::endl << std::flush;

        std::vector<uint32_t> spirv_binary = readFile(filePath);

        std::string fileName = basename(filePath);
        auto tokens = split(fileName, '.');

        spirv_cross::WebGLSLCompiler glsl(std::move(spirv_binary));

        auto fieldDefMapPerShaderNameRef = std::ref(fieldDefMapPerShaderNameVert);
        if (glsl.get_entry_points_and_stages()[0].execution_model == spv::ExecutionModel::ExecutionModelFragment) {
            fieldDefMapPerShaderNameRef = std::ref(fieldDefMapPerShaderNameFrag);
        }
        auto &fieldDefMapPerShaderName = fieldDefMapPerShaderNameRef.get();


        //Find or create new record for shader
        {
            auto it = fieldDefMapPerShaderName.find(tokens[0]);
            if (it == fieldDefMapPerShaderName.end()) {
                fieldDefMapPerShaderName[tokens[0]] = {};
            }
        }

        {
            auto it = shaderMetaInfo.find(fileName);
            if (it == shaderMetaInfo.end()) {
                shaderMetaInfo[fileName] = {};
            }
        }
        auto &perSetMap = fieldDefMapPerShaderName.at(tokens[0]);
        auto &metaInfo = shaderMetaInfo.at(fileName);

        if (glsl.get_entry_points_and_stages()[0].execution_model == spv::ExecutionModel::ExecutionModelVertex) {
            auto it = attributesPerShaderName.find(tokens[0]);
            if (it == attributesPerShaderName.end()) {
                attributesPerShaderName[tokens[0]] = {};
            }
            auto &shaderAttributeVector = attributesPerShaderName.at(tokens[0]);


            auto inputAttributes = glsl.get_shader_resources();
            for (auto &attribute : inputAttributes.stage_inputs) {
                //Create a record if it didnt exist yet

                auto location = glsl.get_decoration(attribute.id, spv::DecorationLocation);

                shaderAttributeVector.push_back({attribute.name, location});
            }

            std::sort(shaderAttributeVector.begin(), shaderAttributeVector.end(),
                      [](const attributeDefine &a, const attributeDefine &b) -> bool {
                          return a.location < b.location;
                      });
        }

        // The SPIR-V is now parsed, and we can perform reflection on it.
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        //Record data for UBO
        for (auto &resource : resources.uniform_buffers) {
            auto uboType = glsl.get_type(resource.type_id);

            auto typeId_size = glsl.get_declared_struct_size(uboType);

            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            metaInfo.uboBindings.push_back({set, binding, typeId_size});

            if (perSetMap.find(binding) != perSetMap.end()) {
                perSetMap[binding] = {};
            }

            auto &fieldVectorDef = perSetMap[binding];

            for (int j = 0; j < uboType.member_types.size(); j++) {

                auto uboParentType = glsl.get_type(uboType.parent_type);
//                glsl.get_member_name
                auto memberSize = glsl.get_declared_struct_member_size(uboParentType, j);
                auto offset = glsl.type_struct_member_offset(uboParentType, j);
                auto memberName = glsl.get_member_name(uboType.parent_type, j);


                dumpMembers(glsl, fieldVectorDef, uboType.member_types[j],
//                            "_" + std::to_string(resource.id) + "_" + memberName, offset, memberSize);
                glsl.to_name(resource.id) + "_" + memberName, offset, memberSize);
            }
        }

        //Record data for images
        for (auto &resource : resources.sampled_images) {
            unsigned int set = -1;
            unsigned int binding = -1;
            if (glsl.has_decoration(resource.id, spv::DecorationDescriptorSet) &&
                glsl.has_decoration(resource.id, spv::DecorationBinding))
            {
                set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
            }

            metaInfo.imageBindings.push_back({set, binding, resource.name});
            if (set >= 0) {
                metaInfo.imageBindingAmounts[set].start =
                    std::min<unsigned int>(metaInfo.imageBindingAmounts[set].start, binding);
                metaInfo.imageBindingAmounts[set].end =
                    std::max<unsigned int>(metaInfo.imageBindingAmounts[set].end, binding);
            }

//            std::cout << "set = " << set << std::endl;
//            std::cout << "binding = " << binding << std::endl;
//            std::cout << resource.name << std::endl;
//            std::cout << glsl.to_name(resource.base_type_id) << std::endl;
        }
        for (auto &data: metaInfo.imageBindingAmounts ) {
            if (data.start < 16) {
                data.length = data.end - data.start + 1;
            } else {
                data.start = 0;
            }
        }

    }

    //2.1 Create attribute enums
    for (auto it = attributesPerShaderName.begin(); it != attributesPerShaderName.end(); it++) {
        std::cout << "struct " << it->first << " {\n"
                                               "    enum class Attribute {" << std::endl;

        std::cout << "        ";
        for (auto &attributeInfo : it->second) {
            std::cout << "" << attributeInfo.name << " = " << attributeInfo.location << ", ";
        }

        std::cout << it->first << "AttributeEnd" << std::endl;
        std::cout << "    };\n"
                     "};" << std::endl << std::endl;
    }
    std::cout << "std::string loadShader(std::string shaderName);" << std::endl;

    //3.1 cpp only data

    std::cout << "#ifdef SHADERDATACPP" << std::endl;

    //3.2 Dump attribute info
    std::cout << "const std::unordered_map<std::string, std::vector<attributeDefine>> attributesPerShaderName = {"
              << std::endl;

    for (auto it = attributesPerShaderName.begin(); it != attributesPerShaderName.end(); it++) {
        std::cout << "{ \"" << it->first << "\",\n" <<
            "  {" << std::endl;

        for (auto &attributeInfo : it->second) {
            std::cout << "    { \"" << attributeInfo.name << "\", " << attributeInfo.location << "}," << std::endl;
        }

        std::cout << "  }\n"<< "}," << std::endl;
    }
    std::cout << "};" << std::endl << std::endl;

    //Add shader meta
    std::cout << "const std::unordered_map<std::string, shaderMetaData> shaderMetaInfo = { \n";
    for (auto it = shaderMetaInfo.begin(); it != shaderMetaInfo.end(); it++) {
        std::cout << "{ \"" << it->first << "\", \n"<<
            "  {\n";

        //Dump UBO Bindings per shader
        std::cout << "    {\n";
        for (auto subIt = it->second.uboBindings.begin(); subIt != it->second.uboBindings.end(); subIt++) {
            std::cout << "      {" << subIt->set << "," << subIt->binding << "," << subIt->size << "}," << std::endl;
        }
        std::cout << "    },\n";
        //UBO Bindings dump end

        std::cout << "    {\n";
        for (auto &binding : it->second.imageBindings) {
            std::cout << "      {" << binding.set << "," << binding.binding << ", \"" << binding.imageName << "\"}," << std::endl;
        }
        std::cout << "    },\n";

        std::cout << "    {\n";
        std::cout << "      {\n";
        for (auto &bindingAmount : it->second.imageBindingAmounts) {
            if (bindingAmount.length > 0) {
                std::cout << "        {" << bindingAmount.start << "," << bindingAmount.end << ", " << bindingAmount.end << "},"
                          << std::endl;
            } else {
                std::cout << "        {0,0,0}," << std::endl;
            }
        }
        std::cout << "      }\n";
        std::cout << "    }\n";

        std::cout << "  }\n},\n";

    }
    std::cout << "};" << std::endl << std::endl;




    //Dump unfform bufffers info
    auto dumpLambda = [](auto fieldDefMapPerShaderName) {
        for (auto it = fieldDefMapPerShaderName.begin(); it != fieldDefMapPerShaderName.end(); it++) {
            std::cout << "  {\"" << it->first << "\", " << " {" << std::endl;

            for (auto subIt = it->second.begin(); subIt != it->second.end(); subIt++) {
                std::cout << "    {" << std::endl;

                std::cout << "      " << subIt->first << ", {" << std::endl;

                for (auto &fieldDef : subIt->second) {
                    std::cout << "        {"
                              << "\"" << fieldDef.name << ((fieldDef.arraySize > 0) ? "[0]" : "") << "\", "
                              << (fieldDef.isFloat ? "true" : "false") << ", "
                              << fieldDef.offset << ", "
                              << fieldDef.columns << ", "
                              << fieldDef.vecSize << ", "
                              << fieldDef.arraySize << "}," << std::endl;
                }

                std::cout << "      }" << std::endl;

                std::cout << "    }," << std::endl;
            }

            std::cout << "  }}," << std::endl;
        }
    };
    std::cout
        << "const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameVert = {"
        << std::endl;
    dumpLambda(fieldDefMapPerShaderNameVert);
    std::cout << "};" << std::endl;

    std::cout
        << "const  std::unordered_map<std::string, std::unordered_map<int, std::vector<fieldDefine>>> fieldDefMapPerShaderNameFrag = {"
        << std::endl;
    dumpLambda(fieldDefMapPerShaderNameFrag);
    std::cout << "};" << std::endl;

    std::cout << "#endif" << std::endl << std::endl;


    std::cout << std::endl << "#endif" << std::endl;
}

#endif //AWEBWOWVIEWERCPP_DUMPSHADERFIELDS_H
