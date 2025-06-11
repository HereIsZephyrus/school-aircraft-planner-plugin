#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices = 100) out;
in vec4 geoColor[];
out vec4 fragColor;
const float PI = 3.14159;
const int resolution = 40;
uniform float radius;
void main() {
    fragColor = geoColor[0];
    vec4 center =  gl_in[0].gl_Position;
    gl_Position = center;
    EmitVertex();
    vec2 offset = vec2(1.0,0.0) * radius;
    float dangle = - 2 * PI / resolution;
    mat2 angleMat = mat2(
                    cos(dangle), -sin(dangle),
                    sin(dangle),  cos(dangle));
    for (int i = 0; i < resolution; i++){
        gl_Position = center + vec4(offset.x,offset.y, 0.0, 0.0);
        EmitVertex();
        if (i%2==1){
            gl_Position = center;
            EmitVertex();
        }
        offset = angleMat * offset;
    }
    gl_Position = center + vec4(offset.x,offset.y, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
}