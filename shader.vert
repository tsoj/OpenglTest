#version 450

in layout(location = 0) vec3 inPosition;

out gl_PerVertex
{
  vec4 gl_Position;
};

void main()
{
  gl_Position = vec4(inPosition, 1.0);
}
