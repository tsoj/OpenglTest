#version 450

in layout(location = 0) vec4 modelPosition;
in layout(location = 1) vec3 modelNormal;
in layout(location = 2) vec2 textureCoordinate;
in layout(location = 3) vec3 modelTangent;

out layout(location = 0) vec3 worldPosition;
out layout(location = 1) vec3 worldNormal;
out layout(location = 2) vec2 fragmentTextureCoordinate;

uniform mat4 modelToWorld;
uniform mat4 worldToProjection;

void main()
{

  worldNormal = normalize(vec3(modelToWorld * vec4(modelNormal, 0.0)));
  vec3 worldTangent = normalize(vec3(modelToWorld * vec4(modelTangent, 0.0)));
  vec3 worldBiTangent = normalize(cross(worldNormal, worldTangent));
  mat3 worldToTangentSpace = mat3(
      vec3(1, 0, 0),
      vec3(0, 1, 0),
      vec3(0, 0, 1)
    );


  worldPosition = vec3(modelToWorld * modelPosition);
  gl_Position = worldToProjection * vec4(worldPosition, 1.0);



  fragmentTextureCoordinate = textureCoordinate;
}
