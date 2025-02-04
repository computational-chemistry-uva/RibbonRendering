#version 330 core
layout (location = 0) in vec3 in_aPos;
layout (location = 1) in vec3 in_bPos;
layout (location = 2) in vec3 in_aCutPlaneNormal;
layout (location = 3) in vec3 in_bCutPlaneNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float cylinderRadius;
uniform int cylinderMode;

out vec3 fPos;
out vec3 fCol;
out vec3 bCoord; // Barycentric coordinates, for wireframe shader
out vec3 fOrigin;
out vec3 fA;
out vec3 fB;
out vec3 startDir;
out vec3 fACPN;
out vec3 fBCPN;

vec3 OFFSETS[18] = vec3[](
    // A cap
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    // Main quad
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    // B cap
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0)
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
    int vID = gl_VertexID % 18;

    // TODO Apply view transform at start?
    vec3 aPos = vec3(model * vec4(in_aPos, 1.0));
    vec3 bPos = vec3(model * vec4(in_bPos, 1.0));
    vec3 aCPN = normalize(transpose(inverse(mat3(model))) * in_aCutPlaneNormal);
    vec3 bCPN = normalize(transpose(inverse(mat3(model))) * in_bCutPlaneNormal);
    vec3 v = 0.5 * (bPos - aPos);
    vec3 centerPos = aPos + v;
    vec3 u = -normalize(cross(v, centerPos - viewPos)) * cylinderRadius;
    vec3 worldNormal = normalize(cross(u, v));
    if (dot(worldNormal, centerPos - viewPos) > 0.0) {
        worldNormal = -worldNormal;
    }

    fA = vec3(view * vec4(aPos, 1.0));
    fB = vec3(view * vec4(bPos, 1.0));
    fACPN = normalize(transpose(inverse(mat3(view))) * aCPN);
    fBCPN = normalize(transpose(inverse(mat3(view))) * bCPN);

    vec3 coords = OFFSETS[vID];

    // Render cap quad that is closer to camera
    // NOTE This optimization allows for only 12 vertices per impostor instead of 18
    //      But we can't do it for the general case because cut planes could both face the camera
    //      Could do this for helices though
    //if (dot(centerPos - viewPos, v) < 0.0) {
    //    coords.xy *= -1.0;
    //}

    vec3 cutPos;
    vec3 cutPlaneNormal;
    if (coords.y < 0.0) {
        cutPos = aPos;
        cutPlaneNormal = aCPN;
    } else {
        cutPos = bPos;
        cutPlaneNormal = bCPN;
    }

    // Calculate vertex position for simple cylinder
    //vec3 pos = centerPos + u * coords.x + coords.y * v + worldNormal * coords.z * cylinderRadius;
    // Calculate vertex position using line intersection with cut plane
    vec3 pos = centerPos + u * coords.x + worldNormal * coords.z * cylinderRadius;
    float d = dot(cutPos - pos, cutPlaneNormal) / dot(normalize(v), cutPlaneNormal);
    pos = pos + d * normalize(v);

    // Extend bounds if drawing sphere end caps
    if (cylinderMode == 1) pos += coords.y * normalize(v) * cylinderRadius;

    gl_Position = projection * view * vec4(pos, 1.0);
    fPos = vec3(view * vec4(pos, 1.0));
    fCol = vec3(1.0);
    bCoord = BARYCENTRIC[vID % 6];

    vec3 _startDir = vec3(1.0, 0.0, 0.0); // TODO Start dir as vertex attribute?
    startDir = normalize(transpose(inverse(mat3(view))) * _startDir);
}
