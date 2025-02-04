#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float sphereRadius;
uniform int raytraced;

out vec3 fPos;
out vec3 fNorm;
out vec3 fCol;
out vec2 fCoord;
out vec3 bCoord; // Barycentric coordinates, for wireframe shader
out vec3 fOrigin;

vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0)
);

vec3 BARYCENTRIC[6] = vec3[](
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0),
    vec3(1.0, 0.0, 0.0)
);

void main() {
    int vID = gl_VertexID % 6;

    vec3 viewPos = vec3(0.0);
    vec3 originPos = vec3(view * model * vec4(aPos, 1.0));
    vec3 normal = normalize(viewPos - originPos);
    vec3 viewUp = normalize(transpose(inverse(mat3(view))) * vec3(0.0, 1.0, 0.0));
    vec3 u = cross(normal, viewUp);
    vec3 v = cross(normal, u);
    vec2 coords = OFFSETS[vID];
    vec3 pos = originPos + coords.x * sphereRadius * u + coords.y * sphereRadius * v;
    if (raytraced != 0) pos += 0.25 * sphereRadius * normal; // Avoid clipping due to perspective distortion

    gl_Position = projection * vec4(pos, 1.0);
    fPos = pos;
    fNorm = normal;
    fCol = vec3(1.0, 0.5, 0.5);
    fCoord = coords;
    bCoord = BARYCENTRIC[vID];
    fOrigin = originPos;
}
