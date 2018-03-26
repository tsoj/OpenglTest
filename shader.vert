#version 450

in layout(location = 0) vec4 modelPosition;
in layout(location = 1) vec3 modelNormal;

out layout(location = 0) vec3 worldPosition;
out layout(location = 1) vec3 worldNormal;

uniform mat4 modelToWorld;
uniform mat4 worldToProjection;

void main()
{
  worldPosition = vec3(modelToWorld * modelPosition);

  gl_Position = worldToProjection * vec4(worldPosition, 1.0);

  worldNormal = vec3((modelToWorld * vec4(modelNormal, 0.0)));
}
