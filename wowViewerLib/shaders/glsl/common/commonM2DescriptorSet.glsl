#include "../common/commonLightFunctions.glsl"

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 256
#endif

// Whole model
layout(std140, set=1, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};

layout(std140, set=1, binding=2) uniform modelWideBlockPS {
    InteriorLightParam intLight;
    LocalLight pc_lights[4];
    ivec4 lightCountAndBcHack;
    vec4 interiorExteriorBlend;
};

layout(std140, set=1, binding=3) uniform boneMats {
    mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};

layout(std140, set=1, binding=4) uniform m2Colors {
    vec4 colors[256];
};

layout(std140, set=1, binding=5) uniform textureWeights {
    vec4 textureWeight[16];
};

layout(std140, set=1, binding=6) uniform textureMatrices {
    mat4 textureMatrix[64];
};