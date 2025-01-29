#version 330 core
out vec4 FragColor;

in vec3 fPos;
//in vec3 fNorm;
in vec3 fCol;
//in vec2 fCoord;
//in vec3 fOrigin;
in vec3 a;
in vec3 b;
in vec3 startDir;
in vec3 faCPN;
in vec3 fbCPN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float lightIntensity;
uniform int drawNormals;
uniform float cylinderRadius;
uniform int distortionCorrection;
uniform int cylinderMode;
uniform float pitch;
uniform float width;

#define PI 3.1415926538

void main() {
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
    float ct = dot(ab, ap) / dot(ab, ab);
    vec3 p = a + ct * ab;
    vec3 normal = normalize(pos - p);

    if (cylinderMode == 2) {
        float nTurns = length(ab) / pitch;
        float angle = nTurns * ct * 2.0 * PI;
        vec3 helixX = normalize(startDir);
        vec3 helixY = normalize(cross(ab, helixX));
        vec3 helixDir = helixX * cos(angle) + helixY * sin(angle);

        if (dot(pos - a, faCPN) < 0.0 || dot(b - pos, fbCPN) < 0.0 || 1.0 - dot(normal, helixDir) > 2.0 * width) {
            pos = max(t0, t1) * d;

            ap = pos - a;
            ct = dot(ab, ap) / dot(ab, ab);

            if (dot(pos - a, faCPN) < 0.0 || dot(b - pos, fbCPN) < 0.0) discard;

            p = a + ct * ab;
            normal = normalize(pos - p);

            float angle = nTurns * ct * 2.0 * PI;
            vec3 helixX = normalize(startDir);
            vec3 helixY = normalize(cross(ab, helixX));
            vec3 helixDir = helixX * cos(angle) + helixY * sin(angle);

            if (1.0 - dot(normal, helixDir) > 2.0 * width) {
                discard;
            }

            normal = -normal;
        }
    }
    else if (cylinderMode == 1) {
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
    else {
        if (dot(pos - a, faCPN) < 0.0 || dot(b - pos, fbCPN) < 0.0) {
            discard;
        }
    }

    vec3 vp = vec3(0.0);
    vec3 lp = vec3(view * vec4(lightPos, 1.0));

    vec3 albedo = fCol;
    //vec3 albedo = fCol * (0.5 + 0.5 * step(0.5, mod(ct * 4.0, 1.0)));
    float diffuse = 1.0;
    float specular = 0.0;
    if (drawNormals != 0) {
        FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    }
    else {
        vec3 viewDir = normalize(vp - pos);
        vec3 lightDir = normalize(lp - pos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);
        float ambient = 0.1;
        diffuse = max(d, 0.0);
        specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        FragColor = vec4(albedo * (ambient + (diffuse + specular) * lightIntensity), 1.0);
    }

    // NOTE writing to depth buffer can decrease performance because it prevents early depth test
    // I can't make it toggleable because setting gl_FragDepth in any branch causes this
    vec4 clipPos = projection * vec4(pos, 1.0);
    float depth = clipPos.z / clipPos.w;
    gl_FragDepth = ((gl_DepthRange.diff * depth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
}
