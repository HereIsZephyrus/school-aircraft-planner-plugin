#version 410 core

in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2DArray textureArray;
uniform int textureID;

void main() {
    fragColor = texture(textureArray, vec3(vTexCoord, textureID));
}