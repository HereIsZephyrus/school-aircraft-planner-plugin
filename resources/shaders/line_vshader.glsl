#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    gl_PointSize = 10.0; // 设置点的大小为10像素
}