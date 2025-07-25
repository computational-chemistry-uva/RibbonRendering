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

uniform vec3 lightPos;
uniform float lightIntensity;
uniform int drawNormals;
uniform float sphereRadius;
uniform int raytraced;

void main() {
    vec3 pos;
    vec3 normal;

    if (raytraced != 0) {
        // Raytrace sphere primitive
        vec3 d = normalize(fPos);
        vec3 s = fOrigin;
        float a = 1.0;
        float b = -2.0 * dot(d, s);
        float c = dot(s, s) - sphereRadius * sphereRadius;
        float discriminant = b * b - 4.0 * a * c;
        if (discriminant < 0.0) {
            discard;
        }
        float t0 = (-b + sqrt(discriminant)) / (2.0 * a);
        float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
        float t = min(t0, t1);
        pos = t * d;
        normal = normalize(pos - fOrigin);
    }
    else {
        // Fake sphere primitive based on texture coordinates
        float distSqr = dot(fCoord, fCoord);
        if (distSqr > 1.0) {
            discard;
        }
        pos = fPos + fNorm * sqrt(1.0 - distSqr) * sphereRadius;
        normal = normalize(pos - fOrigin);
    }

    if (drawNormals != 0) {
        // Color fragment based on normals
        FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    }
    else {
        // Lighting calculations
        vec3 vp = vec3(0.0);
        vec3 lp = lightPos;
        vec3 viewDir = normalize(vp - pos);
        vec3 lightDir = normalize(lp - pos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);

        vec3 albedo = fCol;
        float ambient = 0.1;
        float diffuse = max(d, 0.0);
        float specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        FragColor = vec4(albedo * (ambient + (diffuse + specular) * lightIntensity), 1.0);
    }

    // NOTE Writing to depth buffer can decrease performance because it prevents early depth test
    //      Can't make it toggleable because setting gl_FragDepth in any branch causes this
    vec4 clipPos = projection * vec4(pos, 1.0);
    float depth = clipPos.z / clipPos.w;
    gl_FragDepth = ((gl_DepthRange.diff * depth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}
