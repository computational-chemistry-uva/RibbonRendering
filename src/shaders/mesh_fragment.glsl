#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;
in vec3 fCol;

uniform mat4 view;

uniform vec3 lightPos;
uniform float lightIntensity;
uniform int drawNormals;

void main() {
    if (drawNormals != 0) {
        // Color fragment based on normals
        FragColor = vec4(fNorm * 0.5 + 0.5, 1.0);
    }
    else {
        // Lighting calculations
        vec3 normal = normalize(fNorm);
        vec3 vp = vec3(0.0);
        vec3 lp = lightPos;
        vec3 viewDir = normalize(vp - fPos);
        vec3 lightDir = normalize(lp - fPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);

        vec3 albedo = fCol;
        float ambient = 0.1;
        float diffuse = max(d, 0.0);
        float specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        FragColor = vec4(albedo * (ambient + (diffuse + specular) * lightIntensity), 1.0);
    }
}
