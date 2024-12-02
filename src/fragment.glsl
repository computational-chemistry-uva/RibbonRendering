#version 330 core
out vec4 FragColor;

/* in vec3 FragNormal; */
/* in vec2 FragTexCoords; */
in vec3 fColor;

/* uniform sampler2D diffuseTexture; */
/* uniform vec3 lightColor; */
/* uniform vec3 lightPos; */
/* uniform vec3 viewPos; */

void main() {
    /* // Lighting calculations */
    /* vec3 norm = normalize(FragNormal); */
    /* vec3 lightDir = normalize(lightPos - FragPos); */
    /* float diff = max(dot(norm, lightDir), 0.0); */
    /* vec3 diffuse = diff * lightColor; */
    
    /* // Texture sampling */
    /* vec3 texColor = texture(diffuseTexture, FragTexCoords).rgb; */
    
    /* // Final color */
    /* FragColor = vec4(texColor * diffuse, 1.0); */

    FragColor = vec4(fColor, 1.0);
}
