#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 12) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float cylinderRadius;
/* uniform float time; */

/* in vec3 vPos[]; */
/* in vec3 vNorm[]; */

out vec3 fPos;
//out vec3 fNorm;
out vec3 fCol;
//out vec2 fCoord;
out vec3 bCoord; // Barycentric coordinates, for wireframe shader
out vec3 fOrigin;
out vec3 a;
out vec3 b;
out vec3 startDir;

void main() {
    vec3 aPos = vec3(model * gl_in[0].gl_Position);
    vec3 bPos = vec3(model * gl_in[1].gl_Position);
    vec3 v = 0.5 * (bPos - aPos);
    vec3 centerPos = aPos + v;
    vec3 u = -normalize(cross(v, centerPos - viewPos)) * cylinderRadius;
    vec3 worldNormal = normalize(cross(u, v));
    if (dot(worldNormal, centerPos - viewPos) > 0.0) {
        worldNormal = -worldNormal;
    }

    a = vec3(view * vec4(aPos, 1.0));
    b = vec3(view * vec4(bPos, 1.0));

    vec3 offsets[8] = vec3[](
        vec3(-1.0, -1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3(-1.0, -1.0, 1.0),
        vec3( 1.0, -1.0, 1.0),
        vec3(-1.0,  1.0, 1.0),
        vec3( 1.0,  1.0, 1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3( 1.0,  1.0, -1.0)
    );

    vec3 barycentric[4] = vec3[](
        vec3(0.0, 0.0, 1.0),
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    // Could omit the first or last two verts depending on which side is facing the camera, but backface culling also takes care of this
    // NOTE Only doing this in separate primitives for the wireframe shader. Could all be one loop
    for (int i = 0; i < 4; i++) {
        vec3 coords = offsets[i];
        vec3 pos = centerPos + u * coords.x + coords.y * normalize(v) * (length(v) + cylinderRadius) + worldNormal * coords.z * cylinderRadius;
        gl_Position = projection * view * vec4(pos, 1.0);
        fPos = pos;
        //if (i < 2) fNorm = -normalize(v);
        //else if (i < 6) fNorm = worldNormal;
        //else fNorm = normalize(v);
        fCol = vec3(1.0);
        //fCoord = coords.xy;
        bCoord = barycentric[i];
        //fOrigin = centerPos;

        fPos = vec3(view * vec4(fPos, 1.0));
        //fNorm = normalize(transpose(inverse(mat3(view))) * fNorm);
        //fOrigin = vec3(view * vec4(fOrigin, 1.0));
        startDir = normalize(transpose(inverse(mat3(view))) * worldNormal);

        EmitVertex();
    }
    EndPrimitive();
    for (int i = 2; i < 6; i++) {
        vec3 coords = offsets[i];
        vec3 pos = centerPos + u * coords.x + coords.y * normalize(v) * (length(v) + cylinderRadius) + worldNormal * coords.z * cylinderRadius;
        gl_Position = projection * view * vec4(pos, 1.0);
        fPos = pos;
        //if (i < 2) fNorm = -normalize(v);
        //else if (i < 6) fNorm = worldNormal;
        //else fNorm = normalize(v);
        fCol = vec3(1.0);
        //fCoord = coords.xy;
        bCoord = barycentric[i - 2];
        //fOrigin = centerPos;

        fPos = vec3(view * vec4(fPos, 1.0));
        //fNorm = normalize(transpose(inverse(mat3(view))) * fNorm);
        //fOrigin = vec3(view * vec4(fOrigin, 1.0));
        startDir = normalize(transpose(inverse(mat3(view))) * worldNormal);

        EmitVertex();
    }
    EndPrimitive();
    for (int i = 4; i < 8; i++) {
        vec3 coords = offsets[i];
        vec3 pos = centerPos + u * coords.x + coords.y * normalize(v) * (length(v) + cylinderRadius) + worldNormal * coords.z * cylinderRadius;
        gl_Position = projection * view * vec4(pos, 1.0);
        fPos = pos;
        //if (i < 2) fNorm = -normalize(v);
        //else if (i < 6) fNorm = worldNormal;
        //else fNorm = normalize(v);
        fCol = vec3(1.0);
        //fCoord = coords.xy;
        bCoord = barycentric[i - 4];
        //fOrigin = centerPos;

        fPos = vec3(view * vec4(fPos, 1.0));
        //fNorm = normalize(transpose(inverse(mat3(view))) * fNorm);
        //fOrigin = vec3(view * vec4(fOrigin, 1.0));
        startDir = normalize(transpose(inverse(mat3(view))) * worldNormal);

        EmitVertex();
    }
    EndPrimitive();
}
