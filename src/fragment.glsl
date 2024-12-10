#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform int renderMode;

void main() {
    float diffuse = 1.0;
    float specular = 0.0;
    if (renderMode == 0) {
        vec3 normal = normalize(fNorm);
        vec3 viewDir = normalize(viewPos - fPos);
        vec3 lightDir = normalize(lightPos - fPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);
        diffuse = max(d, (min(d, 0.0) + 1.0) * 0.35);
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    }
    vec3 albedo = vec3(1.0);
    if (renderMode == 1) {
        albedo = fNorm * 0.5 + 0.5;
    }
    FragColor = vec4(albedo * (diffuse + specular), 1.0);
}
