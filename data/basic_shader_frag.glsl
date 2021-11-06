// Specify which version of the GLSL shading language we are using
#version 460

in vec3 norm;
// final pixel output color
out vec4 color;

void main() {
    vec3 to_light = normalize(vec3(-1, -1, -1));
    float n_dot_l = max(0, dot(to_light, norm));
    color = vec4(n_dot_l, 0, 0, 1);  // make the pixel black
}