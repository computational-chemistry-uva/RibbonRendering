#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;
in vec3 fCol;
in vec2 fCoord;
in vec3 fOrigin;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform int renderMode;
uniform int showNormals;
uniform float quadSize;

void main() {
    float distSqr = dot(fCoord, fCoord);
    if (distSqr > 1.0) {
        discard;
    }

    vec3 pos = fPos + fNorm * sqrt(1.0 - distSqr) * 0.5 * quadSize;
    vec3 normal = normalize(pos - fOrigin);

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
        /* diffuse = max(d, (min(d, 0.0) + 1.0) * 0.35); */
        diffuse = mix(0.1, 1.0, max(d, 0.0));
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    }

    float depth = vec4(projection * view * vec4(pos, 1.0)).z;
    depth = (depth - 0.1) / (100.0 - 0.1); // TODO
    gl_FragDepth = depth;
    FragColor = vec4(albedo * (diffuse + specular), 1.0);
}
