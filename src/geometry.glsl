#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

/* uniform float time; */

in vec3 vPos[];
in vec3 vNorm[];

out vec3 fPos;
out vec3 fNorm;

void main() {
    for (int i = 0; i < 3; i++) {
        vec4 pos = gl_in[i].gl_Position;
        gl_Position = pos;
        fPos = vPos[i];
        fNorm = vNorm[i];
        EmitVertex();
    }
    EndPrimitive();
}
