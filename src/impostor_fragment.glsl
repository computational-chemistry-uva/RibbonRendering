#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;
in vec3 fCol;
in vec2 fCoord;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform int renderMode;
uniform int showNormals;

void main() {
    float dist = sqrt(dot(fCoord, fCoord));
    if (dist > 1.0) {
        discard;
    }

    vec3 pos = fPos;
    vec3 normal = normalize(fNorm);

    vec3 albedo = fCol;
    float diffuse = 1.0;
    float specular = 0.0;
    if (showNormals != 0) {
        albedo = normal * 0.5 + 0.5;
    }
    else {
        vec3 viewDir = normalize(viewPos - pos);
        vec3 lightDir = normalize(lightPos - pos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);
        diffuse = max(d, (min(d, 0.0) + 1.0) * 0.35);
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    }
    FragColor = vec4(albedo * (diffuse + specular), 1.0);
}
