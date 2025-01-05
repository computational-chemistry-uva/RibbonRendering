#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float cylinderRadius;
uniform float cylinderExtraQuadLengthFactor;
/* uniform float time; */

/* in vec3 vPos[]; */
/* in vec3 vNorm[]; */

out vec3 fPos;
out vec3 fNorm;
out vec3 fCol;
out vec2 fCoord;
out vec3 fOrigin;
out vec3 a;
out vec3 b;

void main() {
    vec3 aPos = vec3(model * gl_in[0].gl_Position);
    vec3 bPos = vec3(model * gl_in[1].gl_Position);
    vec3 v = 0.5 * (bPos - aPos);
    vec3 centerPos = aPos + v;
    vec3 u = normalize(cross(v, centerPos - viewPos)) * cylinderRadius;
    vec3 worldNormal = normalize(cross(u, v));
    if (dot(worldNormal, centerPos - viewPos) > 0.0) {
        worldNormal = -worldNormal;
    }
    a = vec3(view * vec4(aPos, 1.0));
    b = vec3(view * vec4(bPos, 1.0));
    vec2 offsets[4] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
    );

    for (int i = 0; i < 4; i++) {
        vec2 coords = offsets[i];
        vec3 pos = centerPos + coords.x * u + coords.y * v + coords.y * normalize(v) * cylinderRadius * cylinderExtraQuadLengthFactor;
        gl_Position = projection * view * vec4(pos, 1.0);
        fPos = pos;
        fNorm = worldNormal;
        fCol = vec3(1.0);
        fCoord = coords * cylinderExtraQuadLengthFactor;
        fOrigin = centerPos;

        fPos = vec3(view * vec4(fPos, 1.0));
        fNorm = normalize(transpose(inverse(mat3(view))) * fNorm);
        fOrigin = vec3(view * vec4(fOrigin, 1.0));

        EmitVertex();
    }
    EndPrimitive();
}
