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
uniform sampler2D lightmap;

void main() {
    if (drawNormals != 0) {
        // Color fragment based on normals
        FragColor = vec4(fNorm * 0.5 + 0.5, 1.0);
    }
    else {
        // Lighting calculations
        vec3 normal = normalize(fNorm);
        vec3 vp = vec3(0.0);
        vec3 lp = lightPos;
        vec3 viewDir = normalize(vp - fPos);
        vec3 lightDir = normalize(lp - fPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float d = dot(normal, lightDir);

        //int u = int(mod(fTexCoord.x * 2.0, 1.0) > 0.5);
        //int v = int(mod(fTexCoord.y * 32.0, 1.0) > 0.5);
        //float pattern = 0.5 + 0.5 * float(u ^ v);
        vec3 pattern;
        if (drawTexture != 0) {
            pattern = vec4(texture(lightmap, fTexCoord)).rgb;
        }
        else {
            pattern = vec3(0.75);
        }
        //pattern = pattern * 0.9 + 0.1;
        //pattern = vec3(1.0);
        // TODO AO intensity
        vec3 albedo = fCol;
        vec3 ambient = ambientLightIntensity * pattern;
        float diffuse = max(d, 0.0);
        //float specular = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        float specular = 0.0f;
        float alpha = (gl_FrontFacing ? 1.0 : 0.0);
        FragColor = vec4(albedo * (ambient + (diffuse + specular) * lightIntensity), alpha);
    }
}

//#version 150 core
//in vec2 v_texcoord;
//uniform sampler2D lightmap;
//out vec4 o_color;
//
//void main()
//{
//    o_color = vec4(texture(lightmap, v_texcoord).rgb, gl_FrontFacing ? 1.0 : 0.0);
//    //o_color = vec4(texture(lightmap, v_texcoord).rgb, 1.0);
//}
