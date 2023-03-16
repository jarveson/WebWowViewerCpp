//
// Created by Deamon on 22.12.22.
//

#ifndef AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
#define AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H

#include <memory>
#include "../../gapi/interface/IDevice.h"
#include "../../engine/persistance/header/commonFileStructs.h"
#include "materials/IMaterialStructs.h"


PACK(
    struct WMOVertex {
        C3Vector pos;
        C3Vector normal;
        C2Vector textCoordinate;
        C2Vector textCoordinate2;
        C2Vector textCoordinate3;
        C2Vector textCoordinate4;
        CImVector color;
        CImVector color2;
        CImVector colorSecond;
    }
);

PACK(
    struct LiquidVertexFormat {
        mathfu::vec4_packed pos_transp;
        mathfu::vec2_packed uv;
    }
);


class IMapSceneBufferCreate {
public:
    virtual ~IMapSceneBufferCreate() = default;

    virtual HGVertexBufferBindings createWmoVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createM2VAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createWaterVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;
    virtual HGVertexBufferBindings createSkyVAO(HGVertexBuffer vertexBuffer, HGIndexBuffer indexBuffer) = 0;

    virtual HGVertexBuffer createM2VertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createM2IndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createADTVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createADTIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createWMOVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createWMOIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createWaterVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createWaterIndexBuffer(int sizeInBytes) = 0;

    virtual HGVertexBuffer createSkyVertexBuffer(int sizeInBytes) = 0;
    virtual HGIndexBuffer  createSkyIndexBuffer(int sizeInBytes) = 0;


    virtual std::shared_ptr<IM2ModelData> createM2ModelMat(int bonesCount) = 0;
    virtual std::shared_ptr<IM2Material> createM2Material(const std::shared_ptr<IM2ModelData> &m2ModelData,
                                                          const PipelineTemplate &pipelineTemplate,
                                                          const M2MaterialTemplate &m2MaterialTemplate) = 0;

    virtual std::shared_ptr<ISkyMeshMaterial> createSkyMeshMaterial(const PipelineTemplate &pipelineTemplate) = 0;

    virtual HGMesh createMesh(gMeshTemplate &meshTemplate, const HMaterial &material) = 0;
    virtual HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate, const std::shared_ptr<IM2Material> &material, int layer, int priorityPlane) = 0;
};
typedef std::shared_ptr<IMapSceneBufferCreate> HMapSceneBufferCreate;

#endif //AWEBWOWVIEWERCPP_IMAPSCENEBUFFERCREATE_H
