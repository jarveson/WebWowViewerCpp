//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "../GVertexBufferBindings.h"
#include "../GBlpTexture.h"
enum class MeshType {
    eGeneralMesh = 0,
    eAdtMesh = 1,
    eWmoMesh = 2,
    eOccludingQuery = 3,
    eM2Mesh = 4,
    eParticleMesh = 5,
};

class gMeshTemplate {
public:
    gMeshTemplate(HGVertexBufferBindings bindings, HGShaderPermutation shader) : bindings(bindings), shader(shader) {}
    HGVertexBufferBindings bindings;
    HGShaderPermutation shader;
    MeshType meshType = MeshType::eGeneralMesh;

    int8_t triCCW = 1; //counter-clockwise
    bool depthWrite;
    bool depthCulling;
    bool backFaceCulling;
    EGxBlendEnum blendMode;

    uint8_t colorMask = 0xFF;

    int start;
    int end;
    int element;
    unsigned int textureCount;
    std::vector<HGTexture> texture = std::vector<HGTexture>(4, nullptr);
    HGUniformBuffer vertexBuffers[3] = {nullptr,nullptr,nullptr};
    HGUniformBuffer fragmentBuffers[3] = {nullptr,nullptr,nullptr};
};


class GMesh {
    friend class GDevice;

protected:
    explicit GMesh(GDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    virtual ~GMesh();
    inline HGUniformBuffer getVertexUniformBuffer(int slot) {
        return m_vertexUniformBuffer[slot];
    }
    inline HGUniformBuffer getFragmentUniformBuffer(int slot) {
        return m_fragmentUniformBuffer[slot];
    }
    inline EGxBlendEnum getGxBlendMode() { return m_blendMode; }
    inline bool getIsTransparent() { return m_isTransparent; }
    inline MeshType getMeshType() {
        return m_meshType;
    }
    void setRenderOrder(int renderOrder) {
        m_renderOrder = renderOrder;
    };

    void setEnd(int end) {m_end = end; };
protected:
    MeshType m_meshType;
private:
    HGVertexBufferBindings m_bindings;
    HGShaderPermutation m_shader;

    HGUniformBuffer m_vertexUniformBuffer[3] = {nullptr, nullptr, nullptr};
    HGUniformBuffer m_fragmentUniformBuffer[3] = {nullptr, nullptr, nullptr};
    std::vector<HGTexture> m_texture;

    int8_t m_depthWrite;
    int8_t m_depthCulling;
    int8_t m_backFaceCulling;
    int8_t m_triCCW = 1;
    EGxBlendEnum m_blendMode;
    bool m_isTransparent;


    uint8_t m_colorMask = 0;
    int m_renderOrder = 0;
    int m_start;
    int m_end;
    int m_element;

    int m_textureCount;

private:
    GDevice &m_device;
};


#endif //WEBWOWVIEWERCPP_GMESH_H