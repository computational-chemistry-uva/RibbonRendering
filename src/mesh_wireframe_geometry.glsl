#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 5) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
/* uniform float time; */

/* in vec3 vPos[]; */
in vec3 vNorm[];

out vec3 fPos;
out vec3 fNorm;
out vec3 fCol;

void emitVertex(int i, float value) {
    vec4 pos = gl_in[i].gl_Position;
    gl_Position = pos;
    fPos = vec3(pos);
    fNorm = vNorm[i];
    fCol = vec3(value);
    EmitVertex();
}

void main() {
    emitVertex(0, 1.0);
    emitVertex(1, 1.0);
    emitVertex(2, 1.0);
    EndPrimitive();
    emitVertex(0, 0.2);
    emitVertex(2, 0.2);
    EndPrimitive();
}
