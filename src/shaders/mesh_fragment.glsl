#version 330 core
out vec4 FragColor;

in vec3 fPos;
in vec3 fNorm;
in vec2 fTexCoord;
in vec3 fCol;

uniform mat4 view;

uniform vec3 lightPos;
uniform float lightIntensity;
uniform float ambientLightIntensity;
uniform int drawNormals;
uniform int drawTexture;
uniform int checkerboard;
uniform sampler2D lightmap;

void main() {
    if (drawNormals != 0) {
        // Color fragment based on normals
        FragColor = vec4(fNorm * 0.5 + 0.5, 1.0);
    }
    else {
        // Phong shading
        vec3 normal = normalize(fNorm);
        vec3 vp = vec3(0.0);
        vec3 lp = lightPos;
        vec3 viewDir = normalize(vp - fPos);
        vec3 lightDir = normalize(lp - fPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);
        float diffuse = max(d, 0.0);
        //float specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        float specular = 0.0f;

        // Ambient occlusion
        vec3 pattern;
        if (drawTexture != 0) {
            pattern = vec4(texture(lightmap, fTexCoord)).rgb;
        }
        else {
            pattern = vec3(0.75);
        }
        vec3 albedo = fCol;
        if (checkerboard != 0) {
            ivec2 checker = ivec2(fTexCoord * textureSize(lightmap, 0));
            albedo *= (checker.x % 2) ^ (checker.y % 2);
        }
        vec3 ambient = ambientLightIntensity * pattern;

        // For lightmapping, backface culling is turned off
        // See https://github.com/ands/lightmapper
        float alpha = (gl_FrontFacing ? 1.0 : 0.0);

        FragColor = vec4(albedo * (ambient + (diffuse + specular) * lightIntensity), alpha);
    }
}
