#version 450

in layout(location = 0) vec3 position;
in layout(location = 1) vec2 textureCoordinate;
in layout(location = 2) vec3 cameraPosition;
in layout(location = 3) vec3 lightPosition;
in layout(location = 4) vec4 fragmentPositionLightSpace;

layout(location = 0) out vec4 outColor;

const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 10000.0;

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float transparency;
uniform float shininess;

uniform sampler2D texture;
uniform sampler2D normalMap;
uniform sampler2D depthMap;


float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture2D(depthMap, projCoords.xy).r;
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // check whether current frag pos is in shadow
  float shadow = currentDepth - 0.0001 > closestDepth  ? 1.0 : 0.0;

  return shadow;
}

void main()
{
  vec4 textureColor = texture2D(texture, textureCoordinate);

  vec3 normal = normalize(texture2D(normalMap, textureCoordinate).rgb * 2.0 - vec3(1.0, 1.0, 1.0));

  vec3 toLight = normalize(lightPosition - position);
  vec3 toCamera = normalize(cameraPosition - position);

  float lightDistance = distance(lightPosition, position);

  vec3 ambientLight = ambientColor * vec3(0.0, 0.0, 0.0);
  vec3 diffuseLight = diffuseColor * clamp(dot(toLight, normal), 0.0, 1.0);

  vec3 halfDir = normalize(toLight + toCamera);
  float specAngle = clamp(dot(halfDir, normal), 0.0, 1.0);
  vec3 specularLight = specularColor * pow(specAngle, 20.0);

  float shadow = ShadowCalculation(fragmentPositionLightSpace, normal, toLight);

  vec3 finalLight = clamp(ambientLight + (1.0 - shadow) * (diffuseLight + specularLight) * lightColor * clamp(lightPower/pow(lightDistance, 2.0), 0.0, 1.0) , 0.0, 1.0);

  outColor = textureColor * vec4(finalLight,  transparency);
}
