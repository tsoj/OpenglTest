#version 450

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 worldNormal;
in layout(location = 2) vec2 textureCoordinate;
in layout(location = 3) vec3 cameraPosition;
in layout(location = 4) vec3 lightPosition;

out vec4 outColor;

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

  vec3 normal = vec3(0.0, 0.0, 1.0);//normalize(worldNormal);

  vec3 toLight = normalize(lightPosition - position);
  vec3 toCamera = normalize(cameraPosition - position);

  float lightDistance = distance(lightPosition, position);

  vec3 ambientLight = ambientColor * vec3(0.0, 0.0, 0.0);
  vec3 diffuseLight = diffuseColor * clamp(dot(toLight, normal), 0.0, 1.0);

  vec3 halfDir = normalize(toLight + toCamera);
  float specAngle = clamp(dot(halfDir, normal), 0.0, 1.0);
  vec3 specularLight = specularColor * pow(specAngle, 20.0);

  vec3 finalLight = clamp(ambientLight + (diffuseLight + specularLight) * lightColor * clamp(lightPower/pow(lightDistance, 2.0), 0.0, 1.0) , 0.0, 1.0);

  outColor = textureColor * vec4(finalLight,  transparency);
}
