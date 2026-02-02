#version 330 core

uniform mat4 mvp;

in vec3 vertexPosition;

void main()
{
  gl_Position = mvp * vec4(vertexPosition, 1.0);
}
