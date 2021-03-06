#version 410

layout(location = 0) in vec3 in_position;
layout(location = 2) in vec2 in_texcoord;

layout(location = 3) out vec2 f_texcoord;

uniform mat4 u_modelMat;

void main(void) {
    gl_Position = u_modelMat * vec4(in_position, 1.0);

    f_texcoord = in_position.xy * 0.5 + 0.5;
    f_texcoord.y = 1.0 - f_texcoord.y;
}
