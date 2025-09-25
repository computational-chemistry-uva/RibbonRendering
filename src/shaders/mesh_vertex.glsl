#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fPos;
out vec3 fNorm;
out vec2 fTexCoord;
out vec3 fCol;
out vec3 bCoord; // Barycentric coordinates, for wireframe shader

vec3 BARYCENTRIC[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0)
);

void main() {
    vec4 pos = projection * view * model * vec4(aPos, 1.0);
    vec3 vPos = vec3(view * model * vec4(aPos, 1.0));
    gl_Position = pos;
    fPos = vPos;
    fNorm = normalize(transpose(inverse(mat3(view * model))) * aNorm);
    fTexCoord = aTexCoord;
    fCol = vec3(1.0);
    bCoord = BARYCENTRIC[gl_VertexID % 3];
}

//#version 150 core
//in vec3 aPos;
//in vec2 aTexCoord;
//uniform mat4 view;
//uniform mat4 projection;
//out vec2 v_texcoord;
//
//void main()
//{
//    gl_Position = projection * (view * vec4(aPos, 1.0));
//    v_texcoord = aTexCoord;
//}
