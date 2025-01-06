#version 330 core
out vec4 FragColor;

//in vec3 fCol;
in vec3 bCoord;

void main() {
    vec3 fCol = vec3(0.0, 1.0, 0.0);

    // Wireframe using barycentric coordinates
    vec3 edgeDistances = bCoord;
    vec3 screenSpaceWidths = fwidth(edgeDistances);
    edgeDistances /= screenSpaceWidths;

    float minDistance = min(edgeDistances.x, edgeDistances.y);
    if (minDistance < 1.0) {
        FragColor = vec4(fCol, 1.0);
    } else if (edgeDistances.z < 0.5) {
        FragColor = vec4(fCol, 0.5);
    } else {
        discard;
    }
}
