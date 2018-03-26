#version 450

in layout(location = 0) vec3 worldPosition;
in layout(location = 1) vec3 worldNormal;

out vec4 outColor;

vec3 lightPosition = vec3(0.0, 0.0, -10.0);

void main()
{

  vec3 toLight = normalize(lightPosition - worldPosition);

  outColor = vec4(vec3(1.0, 1.0, 1.0) * dot(toLight, worldNormal), 1.0);
}
