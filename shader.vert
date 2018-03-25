#version 450

layout(location = 0) in dvec3 inPosition;

layout(location = 0) out vec4 outColor;

void main()
{
  gl_Position = vec4(inPosition, 1.0);
  outColor = vec4(0.9, 0.1, 0.0, 1.0);
}
