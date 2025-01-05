#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

/* uniform float time; */

in vec3 vPos[];

out vec3 fPos;
out vec3 fNorm;
out vec3 fCol;

void main() {
    vec3 u = vPos[1] - vPos[0];
    vec3 v = vPos[2] - vPos[0];
    vec3 normal = normalize(cross(u, v));
    for (int i = 0; i < 3; i++) {
        vec4 pos = gl_in[i].gl_Position;
        gl_Position = pos;
        fPos = vPos[i];
        fNorm = normal;
        fCol = vec3(1.0);
        EmitVertex();
    }
    EndPrimitive();
}
