#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;
in vec3 fCol;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform int renderMode;
uniform int showNormals;

void main() {
    vec3 albedo = fCol;
    float diffuse = 1.0;
    float specular = 0.0;
    if (renderMode != 0) {
        if (showNormals != 0) {
            albedo = fNorm * 0.5 + 0.5;
        }
        else {
            vec3 normal = normalize(fNorm);
            vec3 viewDir = normalize(viewPos - fPos);
            vec3 lightDir = normalize(lightPos - fPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            float d = dot(normal, lightDir);
            diffuse = max(d, (min(d, 0.0) + 1.0) * 0.35);
            specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        }
    }
    FragColor = vec4(albedo * (diffuse + specular), 1.0);
}
