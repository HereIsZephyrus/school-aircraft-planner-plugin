#version 330 core

uniform vec4 vColor;
out vec4 fragColor;

void main()
{
    fragColor = vColor;
}