#version 450

in layout(location = 0) vec3 modelPosition;
in layout(location = 1) vec3 modelNormal;

out layout(location = 0) vec3 worldPosition;
out layout(location = 1) vec3 worldNormal;

uniform layout(location = 0) mat4 modelToWorld;
uniform layout(location = 1) mat4 worldToProjection;
uniform layout(location = 2) mat4 modelRotation;

void main()
{
  vec4 v;
  v = modelToWorld * vec4(modelPosition, 1.0);
  worldPosition = vec3(v.x, v.y, v.z);

  gl_Position = worldToProjection * v;

  v = modelRotation * vec4(modelNormal, 1.0);
  worldNormal = normalize(vec3(v.x, v.y, v.z));
}
