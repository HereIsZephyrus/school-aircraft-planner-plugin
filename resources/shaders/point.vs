#version 410 core
layout (location = 0) in vec3 aPos;
uniform vec4 vColor;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
out vec3 geoColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    geoColor = vColor;
}