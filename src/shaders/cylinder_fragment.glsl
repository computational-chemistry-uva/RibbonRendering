#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fCol;
in vec3 fA;
in vec3 fB;
in vec3 fStartDir;
in vec3 fACPN;
in vec3 fBCPN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform float lightIntensity;
uniform float ambientLightIntensity;
uniform int drawNormals;
uniform float cylinderRadius;
uniform int distortionCorrection;
uniform int cylinderMode;
uniform float pitch;
uniform float width;

#define PI 3.1415926538

void main() {
    vec3 a = fA;
    vec3 b = fB;

    // Raytrace cylinder primitive
    vec3 d = normalize(fPos);
    vec3 v = normalize(b - a);
    vec3 x = -a;
    float A = 1.0 - dot(v, d) * dot(v, d);
    float B = 2.0 * (dot(d, x) - dot(v, d) * dot(v, x));
    float C = dot(x, x) - dot(v, x) * dot(v, x) - cylinderRadius * cylinderRadius;
    float discriminant = B * B - 4.0 * A * C;
    if (discriminant < 0.0) {
        discard;
    }
    float t0 = (-B + sqrt(discriminant)) / (2.0 * A);
    float t1 = (-B - sqrt(discriminant)) / (2.0 * A);
    float t = min(t0, t1);
    vec3 pos = t * d;

    vec3 ab = b - a;
    vec3 ap = pos - a;
    float ct = dot(ab, ap) / dot(ab, ab); // ct is the length along the cylinder where ray hits, 0 at A, 1 at B
    vec3 p = a + ct * ab; // p is the hit point projected onto the center axis of the cylinder
    vec3 normal = normalize(pos - p);

    if (cylinderMode == 2) { // Helix
        float nTurns = length(ab) / pitch;
        float angle = nTurns * ct * 2.0 * PI;
        vec3 helixX = normalize(fStartDir);
        vec3 helixY = normalize(cross(ab, helixX));
        vec3 helixDir = helixX * cos(angle) + helixY * sin(angle);

        // Check if outside is hit
        // If not using CPNs, check if 0 <= ct <= 1
        if (dot(pos - a, fACPN) < 0.0 || dot(b - pos, fBCPN) < 0.0 || 1.0 - dot(normal, helixDir) > 2.0 * width) {
            // Outside not hit; check if inside is hit
            pos = max(t0, t1) * d;
            ap = pos - a;
            ct = dot(ab, ap) / dot(ab, ab);

            if (dot(pos - a, fACPN) < 0.0 || dot(b - pos, fBCPN) < 0.0) discard;

            p = a + ct * ab;
            normal = normalize(pos - p);

            float angle = nTurns * ct * 2.0 * PI;
            vec3 helixDir = helixX * cos(angle) + helixY * sin(angle);

            if (1.0 - dot(normal, helixDir) > 2.0 * width) {
                // Inside not hit; discard fragment
                discard;
            }

            // Inside is hit; flip normal
            normal = -normal;
        }
    }
    else if (cylinderMode == 1) { // Sphere caps
        if (ct < 0.0 || ct > 1.0) {
            // Raytrace a sphere endcap
            vec3 s;
            if (ct < 0.0) s = a;
            else s = b;
            float A = 1.0;
            float B = -2.0 * dot(d, s);
            float C = dot(s, s) - cylinderRadius * cylinderRadius;
            float discriminant = B * B - 4.0 * A * C;
            if (discriminant < 0.0) {
                discard;
            }
            float t0 = (-B + sqrt(discriminant)) / (2.0 * A);
            float t1 = (-B - sqrt(discriminant)) / (2.0 * A);
            float t = min(t0, t1);
            pos = t * d;
            normal = normalize(pos - s);
        }
    }
    else { // Simple cylinder
        if (dot(pos - a, fACPN) < 0.0 || dot(b - pos, fBCPN) < 0.0) {
            discard;
        }
    }

    if (drawNormals != 0) {
        // Color fragment based on normal
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
        float ambient = ambientLightIntensity;
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
