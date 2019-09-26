#version 450

precision highp float;
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexcoord0;

layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTexcoord0;

void main() {
    vec4 aPositionVec4 = vec4(aPosition, 1);

    vColor = aColor;
    vTexcoord0 = aTexcoord0;
    gl_Position = uPMatrix * uLookAtMat * aPositionVec4;
    gl_Position.y = -gl_Position.y;
}
