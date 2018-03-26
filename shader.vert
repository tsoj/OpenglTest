#version 450

layout(location = 0) in dvec3 inPosition;

layout(location = 0) out vec4 outColor;

layout(location = 0) uniform mat4 modelToWorld;
layout(location = 1) uniform mat4 worldToProjection;

void main()
{
  vec4 v = vec4(inPosition, 1.0);
  vec4 newPosition = worldToProjection*modelToWorld*v;
  gl_Position = newPosition;
  outColor = vec4(0.9, 0.1, 0.0, 1.0);
}
