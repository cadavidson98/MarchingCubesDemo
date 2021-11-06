// specify which version of the GLSL shading language we are using
#version 460

// Input Vertex attributes. Here there is only a 2D screen position
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 norm;

uniform mat4 view;
uniform mat4 proj;

void main() {
    norm = normal;
    gl_Position = proj * (view * vec4(position.x, position.y, position.z, 1.0));
}