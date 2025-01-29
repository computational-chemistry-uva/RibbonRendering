#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float sphereRadius;
uniform int raytraced;

out vec3 fPos;
out vec3 fNorm;
out vec3 fCol;
out vec2 fCoord;
out vec3 bCoord; // Barycentric coordinates, for wireframe shader
out vec3 fOrigin;

// TODO Put offsets in VBO?
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

    vec3 worldPos = vec3(model * vec4(aPos, 1.0));
    vec3 worldNormal = normalize(viewPos - worldPos);
    vec3 right = normalize(cross(worldNormal, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(worldNormal, right);
    vec2 coords = OFFSETS[vID];
    vec3 pos = worldPos + coords.x * sphereRadius * right + coords.y * sphereRadius * up;
    if (raytraced != 0) pos += 0.25 * sphereRadius * worldNormal; // Avoid clipping due to perspective distortion

    gl_Position = projection * view * vec4(pos, 1.0);
    fPos = vec3(view * vec4(pos, 1.0));
    fNorm = normalize(transpose(inverse(mat3(view))) * worldNormal);
    fCol = vec3(1.0, 0.5, 0.5);
    fCoord = coords;
    bCoord = BARYCENTRIC[vID];
    fOrigin = vec3(view * vec4(worldPos, 1.0));
}
