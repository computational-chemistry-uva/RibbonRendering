#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;
in vec3 fCol;
in vec2 fCoord;
in vec3 fOrigin;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform int renderMode;
uniform int showNormals;
uniform float quadSize;
uniform int distortionCorrection;

void main() {
    vec3 pos;
    vec3 normal;
    if (distortionCorrection != 0) {
        vec3 d = normalize(fPos);
        vec3 s = fOrigin;
        float a = 1.0;
        float b = -2.0 * dot(d, s);
        float c = dot(s, s) - (0.5 * quadSize) * (0.5 * quadSize);
        float discriminant = b * b - 4.0 * a * c;
        if (discriminant < 0.0) {
            discard;
        }
        float t0 = (-b + sqrt(discriminant)) / 2.0 * a;
        float t1 = (-b - sqrt(discriminant)) / 2.0 * a;
        float t = min(t0, t1);
        pos = t * d;
        normal = normalize(pos - fOrigin);
    }
    else {
        float distSqr = dot(fCoord, fCoord);
        if (distSqr > 1.0) {
            discard;
        }
        pos = fPos + fNorm * sqrt(1.0 - distSqr) * 0.5 * quadSize;
        normal = normalize(pos - fOrigin);
    }

    /* vec3 vp = vec3(view * vec4(viewPos, 1.0)); */
    vec3 vp = vec3(0.0);
    vec3 lp = vec3(view * vec4(lightPos, 1.0));

    vec3 albedo = fCol;
    float diffuse = 1.0;
    float specular = 0.0;
    if (showNormals != 0) {
        albedo = normal * 0.5 + 0.5;
    }
    else {
        vec3 viewDir = normalize(vp - pos);
        vec3 lightDir = normalize(lp - pos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);
        /* diffuse = max(d, (min(d, 0.0) + 1.0) * 0.35); */
        diffuse = mix(0.1, 1.0, max(d, 0.0));
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    }

    FragColor = vec4(albedo * (diffuse + specular), 1.0);

    // NOTE writing to depth buffer can decrease performance because it prevents early depth test
    // I can't make it toggleable because setting gl_FragDepth in any branch causes this
    vec4 clipPos = projection * vec4(pos, 1.0);
    float depth = clipPos.z / clipPos.w;
    gl_FragDepth = ((gl_DepthRange.diff * depth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}
