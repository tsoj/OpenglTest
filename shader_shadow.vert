#version 450

in layout(location = 0) vec4 modelPosition;

uniform mat4 modelToProjection;

void main()
{
  gl_Position = modelToProjection * modelPosition;
}
