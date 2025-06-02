#version 410 core

in vec3 geoColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(geoColor, 1.0);
}