#version 330 core
layout (location = 0) in vec3 in_aPos;
layout (location = 1) in vec3 in_bPos;
layout (location = 2) in vec3 in_aCutPlaneNormal;
layout (location = 3) in vec3 in_bCutPlaneNormal;
//layout (location = 2) in vec3 in_a1Pos;
//layout (location = 3) in vec3 in_b1Pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float cylinderRadius;
uniform int cylinderMode;

out vec3 fPos;
//out vec3 fNorm;
out vec3 fCol;
//out vec2 fCoord;
out vec3 bCoord; // Barycentric coordinates, for wireframe shader
out vec3 fOrigin;
out vec3 a;
out vec3 b;
out vec3 startDir;
out vec3 faCPN;
out vec3 fbCPN;

// TODO Put offsets in VBO?
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
    //vec3 a1Pos = vec3(model * vec4(in_a1Pos, 1.0));
    //vec3 b1Pos = vec3(model * vec4(in_b1Pos, 1.0));
    //vec3 aCPN = normalize(bPos - a1Pos);
    //vec3 bCPN = normalize(b1Pos - aPos);
    vec3 v = 0.5 * (bPos - aPos);
    vec3 centerPos = aPos + v;
    vec3 u = -normalize(cross(v, centerPos - viewPos)) * cylinderRadius;
    vec3 worldNormal = normalize(cross(u, v));
    if (dot(worldNormal, centerPos - viewPos) > 0.0) {
        worldNormal = -worldNormal;
    }

    a = vec3(view * vec4(aPos, 1.0));
    b = vec3(view * vec4(bPos, 1.0));
    faCPN = normalize(transpose(inverse(mat3(view))) * aCPN);
    fbCPN = normalize(transpose(inverse(mat3(view))) * bCPN);

    vec3 coords = OFFSETS[vID];

    // Render cap quad that is closer to camera
    // TODO Need to draw both sides because of cut planes
    //      This optimization can be done for helices though
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

    //vec3 pos = centerPos + u * coords.x + coords.y * v + worldNormal * coords.z * cylinderRadius;
    vec3 pos = centerPos + u * coords.x + worldNormal * coords.z * cylinderRadius;
    float d = dot(cutPos - pos, cutPlaneNormal) / dot(normalize(v), cutPlaneNormal);
    pos = pos + d * normalize(v);

    if (cylinderMode == 1) pos += coords.y * normalize(v) * cylinderRadius;
    gl_Position = projection * view * vec4(pos, 1.0);
    fPos = pos;
    //if (i < 2) fNorm = -normalize(v);
    //else if (i < 6) fNorm = worldNormal;
    //else fNorm = normalize(v);
    fCol = vec3(1.0);
    //fCoord = coords.xy;
    bCoord = BARYCENTRIC[vID % 6];
    //fOrigin = centerPos;

    //worldNormal = vec3(1.0, 0.0, 0.0);
    //vec3 _startDir = worldNormal;
    vec3 _startDir = vec3(1.0, 0.0, 0.0); // TODO

    fPos = vec3(view * vec4(fPos, 1.0));
    //fNorm = normalize(transpose(inverse(mat3(view))) * fNorm);
    //fOrigin = vec3(view * vec4(fOrigin, 1.0));
    startDir = normalize(transpose(inverse(mat3(view))) * _startDir);
}
