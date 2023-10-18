#ifndef COMMON_WMO_INDIRECT_DS
#define COMMON_WMO_INDIRECT_DS

layout(std430, set=1, binding=1) buffer readonly modelWideBlockVS {
    mat4 uPlacementMats[];
};

layout(std430, set=1, binding=2) buffer readonly meshWideBlockVS {
    ivec4 VertexShader_UseLitColors[];
};

struct WmoMeshWide {
    ivec4 UseLitColor_EnableAlpha_PixelShader_BlendMode;
    vec4 FogColor_AlphaTest;
};

layout(std430, set=1, binding=3) buffer readonly meshWideBlockPS {
    WmoMeshWide wmoMeshWides[];
};

struct WmoMeshWideBindless {
    ivec4 placementMat_meshWideIndex_blockVSIndex_texture9;
    ivec4 text1_text2_text3_text4;
    ivec4 text5_text6_text7_text8;
};

layout(std430, set=1, binding=4) buffer readonly meshWideBlockBindlessPS {
    WmoMeshWideBindless wmoMeshWideBindlesses[];
};

layout(std430, set=1, binding=5) buffer readonly wmoLocalAmbient {
    vec4 s_wmoAmbient[];
};

struct WMOPerMeshData {
    ivec4 meshWideBindlessIndex_wmoAmbientIndex;
};

layout(std430, set=1, binding=6) buffer readonly _wmoPerMeshData {
    WMOPerMeshData perMeshDatas[];
};

#endif //COMMON_WMO_INDIRECT_DS