#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_shader_draw_parameters: require


precision highp float;
precision highp int;

#include "../../../common/commonLightFunctions.glsl"
#include "../../../common/commonFogFunctions.glsl"
#include "../../../common/commonADTMaterial.glsl"

/* vertex shader code */
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec4 aVertexLighting;
layout(location = 3) in vec3 aNormal;

#include "../../../common/commonUboSceneData.glsl"
#include "../../../common/commonAdtIndirectDescriptorSet.glsl"


mat3 blizzTranspose(mat4 value) {
    return mat3(
        value[0].xyz,
        value[1].xyz,
        value[2].xyz
    );
}

layout(location = 0) out vec2 vChunkCoords;
layout(location = 1) out vec3 vPosition;
layout(location = 2) out vec4 vColor;
layout(location = 3) out vec3 vNormal;
layout(location = 4) out vec3 vVertexLighting;
layout(location = 5) out vec2 vAlphaCoords;
layout(location = 6) out flat int vMeshIndex;

void main() {
    AdtInstanceData adtInstanceData = adtInstanceDatas[gl_InstanceIndex];
    AdtMeshWideVSPS adtMeshWideVSPS = adtMeshWideVSPSes[adtInstanceData.meshIndexVSPS_meshIndexPS_AlphaTextureInd.x];

/*
     Y
  X  0    1    2    3    4    5    6    7    8
        9   10   11   12   13   14   15   16
*/

    //With current implementation ADT's VBO is combined buffer, which contains all vertexes from all MCNK of ADT
    //Thus, we first need to get vertexNumber within MCNK

    int indexInMCNK = (gl_VertexIndex - gl_BaseVertexARB) % (9 * 9 + 8 * 8);
    calcAdtAlphaUV(indexInMCNK, adtMeshWideVSPS.uPos.xyz, vAlphaCoords, vChunkCoords);

    vec4 worldPoint = vec4(aPos, 1);

    mat4 viewMatForNormal = transpose(inverse(scene.uLookAtMat));
    vec3 normal = normalize((viewMatForNormal * vec4(aNormal, 0.0)).xyz);

    vPosition = (scene.uLookAtMat * worldPoint).xyz;
    vNormal = normal;
    vColor = aColor;
    vVertexLighting = aVertexLighting.rgb;
    vMeshIndex = gl_InstanceIndex;

    gl_Position = scene.uPMatrix * scene.uLookAtMat * worldPoint;
}