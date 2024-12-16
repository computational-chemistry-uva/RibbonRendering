#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float quadSize;
/* uniform float time; */

/* in vec3 vPos[]; */
/* in vec3 vNorm[]; */

out vec3 fPos;
out vec3 fNorm;
out vec3 fCol;
out vec2 fCoord;
out vec3 fOrigin;

void main() {
    vec3 viewRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 viewUp = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 worldPos = vec3(model * gl_in[0].gl_Position);
    vec3 worldNormal = normalize(cross(viewRight, viewUp));

    vec2 offsets[4] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
    );

    for (int i = 0; i < 4; i++) {
        vec2 coords = offsets[i];
        vec3 pos = worldPos + coords.x * 0.5 * quadSize * viewRight + coords.y * 0.5 * quadSize * viewUp;
        gl_Position = projection * view * vec4(pos, 1.0);
        fPos = vec3(pos);
        fNorm = worldNormal;
        fCol = vec3(1.0);
        fCoord = coords;
        fOrigin = worldPos;
        EmitVertex();
    }
    EndPrimitive();
}
