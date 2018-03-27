#version 450

in layout(location = 0) vec3 worldPosition;
in layout(location = 1) vec3 worldNormal;
in layout(location = 2) vec2 textureCoordinate;

out vec4 outColor;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 10000.0;

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float transparency;
uniform float shininess;

uniform sampler2D texture;

void main()
{
  vec4 textureColor = texture2D(texture, textureCoordinate);

  vec3 normal = normalize(worldNormal);

  vec3 toLight = normalize(lightPosition - worldPosition);
  vec3 toCamera = normalize(cameraPosition - worldPosition);

  float lightDistance = distance(lightPosition, worldPosition);

  vec3 ambientLight = ambientColor * vec3(0.2, 0.2, 0.2);
  vec3 diffuseLight = diffuseColor * clamp(dot(toLight, normal), 0.0, 1.0);

  vec3 halfDir = normalize(toLight + toCamera);
  float specAngle = clamp(dot(halfDir, normal), 0.0, 1.0);
  vec3 specularLight = specularColor * pow(specAngle, 20.0);

  vec3 finalLight = clamp(ambientLight + (diffuseLight + specularLight) * lightColor * clamp(lightPower/pow(lightDistance, 2.0), 0.0, 1.0) , 0.0, 1.0);

  outColor = textureColor * vec4(finalLight,  1.0);
}
