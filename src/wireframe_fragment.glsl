#version 330 core
out vec4 FragColor;

in vec3 fCol;

void main() {
    FragColor = vec4(fCol, 1.0);
}
