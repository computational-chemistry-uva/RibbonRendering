#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

/* uniform float time; */

/* in vec3 Normal[]; */
/* in vec2 TexCoords[]; */
in vec3 vColor[];

/* out vec3 FragNormal; */
/* out vec2 FragTexCoords; */
out vec3 fColor;

void main() {
    for (int i = 0; i < 3; i++) {
        // Extrude along normal
        vec4 pos = gl_in[i].gl_Position;
        /* vec3 normal = Normal[i]; */
        
        gl_Position = pos;
        /* FragNormal = normal; */
        /* FragTexCoords = TexCoords[i]; */
        fColor = vColor[i];
        EmitVertex();
        
        /* gl_Position = pos + vec4(normal * sin(time), 0.0); */
        /* FragNormal = normal; */
        /* FragTexCoords = TexCoords[i]; */
        /* EmitVertex(); */
    }
    EndPrimitive();
}
