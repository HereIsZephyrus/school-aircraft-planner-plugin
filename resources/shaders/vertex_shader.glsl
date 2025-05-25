#version 330 core

layout(location = 0) in vec3 aPos;  // 顶点位置
uniform mat4 projection;             // 投影矩阵
uniform mat4 modelview;              // 模型视图矩阵

void main()
{
    gl_Position = projection * modelview * vec4(aPos, 1.0);  // 变换顶点坐标
}
