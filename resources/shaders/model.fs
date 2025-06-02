#version 410 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D textureSampler;

void main()
{

   fragColor = texture(textureSampler, vTexCoord);
}