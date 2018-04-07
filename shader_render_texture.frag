#version 450

out vec4 outColor;

in layout(location = 0) vec2 textureCoordinate;

uniform sampler2D texture;

void main()
{
  vec4 textureColor = texture2D(texture, textureCoordinate);
  outColor = textureColor;
}
