#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;
in vec3 fCol;

uniform mat4 view;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float lightIntensity;
uniform int drawNormals;

void main() {
    vec3 albedo = fCol;
    float diffuse = 1.0;
    float specular = 0.0;
    if (drawNormals != 0) {
        FragColor = vec4(fNorm * 0.5 + 0.5, 1.0);
    }
    else {
        vec3 vp = vec3(0.0);
        vec3 lp = vec3(view * vec4(lightPos, 1.0));
        vec3 normal = normalize(fNorm);
        vec3 viewDir = normalize(vp - fPos);
        vec3 lightDir = normalize(lp - fPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);
        //diffuse = max(d, (min(d, 0.0) + 1.0) * 0.35);
        float ambient = 0.1;
        diffuse = max(d, 0.0);
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        FragColor = vec4(albedo * (ambient + (diffuse + specular) * lightIntensity), 1.0);
    }
}
