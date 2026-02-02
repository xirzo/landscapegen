#version 330 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;
uniform float seconds;

float sin_wave(vec2 v, float amplitude, float wave_x, float wave_y)
{
    return amplitude * sin(wave_x * v.x + wave_y * v.y + seconds);
}

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    vec2 pos = vertexPosition.xz;
    
    float displacement = sin_wave(pos, 5.0, 0.05, 0.05);

    vec3 newPos = vec3(vertexPosition.x, vertexPosition.y + displacement, vertexPosition.z);

    gl_Position = mvp * vec4(newPos, 1.0);
}
