#version 450

in layout(location = 0) vec3 modelPosition;

out layout(location = 0) vec4 outColor;

uniform layout(location = 0) mat4 modelToView;
uniform layout(location = 1) mat4 worldToProjection;

void main()
{
  vec4 worldPosition = modelToView * vec4(modelPosition, 1.0);
  vec4 projectedWorldPosition = worldToProjection * worldPosition;
  gl_Position = projectedWorldPosition;
  outColor = vec4(0.9, 0.1, 0.0, 1.0);
}
