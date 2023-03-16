//
// Created by Deamon on 7/22/2018.
//

#ifndef AWEBWOWVIEWERCPP_GM2MESH_H
#define AWEBWOWVIEWERCPP_GM2MESH_H

#include "GMeshVLK.h"

class GM2MeshVLK : public GMeshVLK, public IM2Mesh {
public:
    GM2MeshVLK(const gMeshTemplate &meshTemplate, const HMaterialVLK &material);

public:
    void setLayer(int layer) override;
    void setPriorityPlane(int priorityPlane) override;
    void setSortDistance(float distance) override;
    float getSortDistance() override;
    EGxBlendEnum getGxBlendMode() override;
};
#endif //AWEBWOWVIEWERCPP_GM2MESH_H
