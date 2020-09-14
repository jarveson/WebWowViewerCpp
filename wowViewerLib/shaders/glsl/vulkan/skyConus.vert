#version 450

#extension GL_GOOGLE_include_directive: require

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"

precision highp float;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};
layout(std140, set=0, binding=2) uniform meshWideBlockVS {
    vec4 skyColor[6];
};

layout(location = 0) in vec4 aPosition;

layout(location = 0) out vec4 vColor;



void main() {


    vec3 inputPos = aPosition.xyz;
//Correction of conus
    inputPos.xy = inputPos.xy / 0.6875;
    inputPos.z = inputPos.z > 0 ? (inputPos.z / 0.2928): inputPos.z;

//    inputPos.z = -1.0-inputPos.z;
    vec4 cameraPos = scene.uLookAtMat * vec4(inputPos.xyz, 1.0);
    cameraPos.xyz = cameraPos.xyz - scene.uLookAtMat[3].xyz;
    cameraPos.z = cameraPos.z ;

    vColor = vec4(skyColor[int(aPosition.w)].xyz, 1.0);
	gl_Position = scene.uPMatrix * cameraPos;
}
