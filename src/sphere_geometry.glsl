#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float sphereRadius;
uniform float sphereQuadSizeFactor;
/* uniform float time; */

/* in vec3 vPos[]; */
/* in vec3 vNorm[]; */

out vec3 fPos;
out vec3 fNorm;
out vec3 fCol;
out vec2 fCoord;
out vec3 bCoord; // Barycentric coordinates, for wireframe shader
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

    vec3 barycentric[4] = vec3[](
        vec3(0.0, 0.0, 1.0),
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    for (int i = 0; i < 4; i++) {
        vec2 coords = offsets[i];
        vec3 pos = worldPos + coords.x * sphereRadius * sphereQuadSizeFactor * viewRight + coords.y * sphereRadius * sphereQuadSizeFactor * viewUp;
        gl_Position = projection * view * vec4(pos, 1.0);
        fPos = pos;
        fNorm = worldNormal;
        fCol = vec3(1.0, 0.5, 0.5);
        fCoord = coords * sphereQuadSizeFactor;
        bCoord = barycentric[i];
        fOrigin = worldPos;

        fPos = vec3(view * vec4(fPos, 1.0));
        fNorm = normalize(transpose(inverse(mat3(view))) * fNorm);
        fOrigin = vec3(view * vec4(fOrigin, 1.0));

        EmitVertex();
    }
    EndPrimitive();
}
