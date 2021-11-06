#include "config.h"

#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include "marching_cubes.h"

using namespace std;

string load_shader_source(string file_name) {
    ifstream shaderFile(file_name);
    if(!shaderFile.good()) exit(1);
	return string((istreambuf_iterator<char>(shaderFile)), istreambuf_iterator<char>());
}

GLuint init_shader() {
    GLuint shader, vert, frag;
    string v_str = load_shader_source(CUR_DIR+string("/basic_shader.glsl")).c_str();
    string f_str = load_shader_source(CUR_DIR+string("/basic_shader_frag.glsl")).c_str();
    
    const char* v_src = v_str.c_str();
    const char* f_src = f_str.c_str();
    
    shader = glCreateProgram();
    vert = glCreateShader(GL_VERTEX_SHADER);
    frag = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(vert, 1, &v_src, NULL);
    glShaderSource(frag, 1, &f_src, NULL);

    glCompileShader(vert);
    GLint status;
	glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
	if (!status){
		char buffer[512];
		glGetShaderInfoLog(frag, 512, NULL, buffer);
		printf("Vertex Shader Compile Failed. Info:\n\n%s\n",buffer);
	}
    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
	if (!status){
		char buffer[512];
		glGetShaderInfoLog(frag, 512, NULL, buffer);
		printf("Vertex Shader Compile Failed. Info:\n\n%s\n",buffer);
	}
    glAttachShader(shader, vert);
    glAttachShader(shader, frag);

    glBindFragDataLocation(shader, 0, "color");
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetProgramInfoLog(shader, 512, NULL, buffer);
        printf("Shader Link Failed. Info:\n\n%s\n",buffer);
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
    
    return shader;
};

int main(int argc, char* argv[]) {
    // Initialize GLFW - this enables all further GLFW calls
    if(!glfwInit()) {
        // GLFW could not be initialized
        return 1;
    }
    // Create a window - set some hints so GLFW knows what kind of pixel format
    // to use and which OpenGL context to create
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // the OpenGL major version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);  // the OpenGL minor version
    // which OpenGL functions do we want access to? Do we want to support Legacy OpenGL?
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    // Should depracated OpenGL functions be removed in 3.0+? (This must be set when running on MacOS)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);  // Optional: Allow users to resize the window by pressing a button on the border or dragging the window edges

    // Create the window
    GLFWwindow* window = glfwCreateWindow(1080, 720, "Marching Cubes test", NULL, NULL);
    if (!window) {
        // Window or OpenGL context creation failed
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Could not initalize OpenGL" << endl;
        return 1;
    };
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, 1080, 720);
    
    GLuint shader = init_shader();
    GLuint err = glGetError();
    // Next, we need OpenGL resources to store and process our model vertices
    GLuint vao, vbo;
    // The vertex Array communicates with the vertex shader, describing how
    // each vertex is formatted and how the vertices are stored in memory
    glGenVertexArrays(1, &vao);
    // The vertex buffer is a chunk of memory on the GPU which stores
    // vertices for rendering
    glGenBuffers(1, &vbo);
    // Binding a vertex array ensures that all subsequent OpenGL calls
    // regarding vertex arrays will be applied to the vao variable
    glBindVertexArray(vao);
    // Binding a vertex buffer ensures that all subsequent OpenGL calls
    // regarding vertex buffers will be applied to the vbo variable
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // This is our data.
    // Upload the data to the GPU. We must request a chunk of memory
    // that can store all of our vertices and specify where on the GPU
    // we would like the memory
    MarchingCubes mc(Point(-1.25f, -1.25f, -1.25f), Point(1.25f, 1.25f, 1.25f), .1f);
    vector<Vertex> verts;
    mc.Polygonize(verts);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts.size(), data(verts), GL_STATIC_DRAW);
    // Now we need to configure the vertex array object so vertices are processed
    // by the shader program properly
    GLuint pos_attrib = glGetAttribLocation(shader, "position");
    GLuint normal_attrib = glGetAttribLocation(shader, "normal");
    // specify that each vertex only has one attribute, a position (x,y).
    // the vertices are all contiguous in memory, so the stride (offset between vertices)
    // should be 0
    // As we add more attributes to a vertex, the vertex attribute array must account
    // for the multiple attributes and informing the shader about how to process each
    // attribute.
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
    glEnableVertexArrayAttrib(vao, pos_attrib);
    
    glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
    glEnableVertexArrayAttrib(vao, normal_attrib);
    // set the OpenGL window background color to gray
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
 
    // set the camera using GLM
    glm::mat4 proj = glm::infinitePerspective(3.14f/4.f, 1080.f/720.f, .1f);
    glm::mat4 view = glm::lookAt(glm::vec3(3, 3, 3),
                                 glm::vec3(0, 0, 0),
                                 glm::vec3(0, 1, 0));
    // make sure the vertex attribute array and vertex buffers are bound for
    // drawing calls
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);    
    // Enable the shader for drawing commands
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    
    // The while loop ensures that our program will continue running until the
    // user presses the "close" icon
    while(!glfwWindowShouldClose(window)) {
        // check for user input (mouse, keyboard, video game controller, etc.)
        glfwPollEvents();
        // clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        // draw the vertices stored in the vertex buffer. We must tell the shader
        // how many vertices we have (3) and how to connect the vertices (i.e a triangle)
        glDrawArrays(GL_TRIANGLES, 0, verts.size());
        // update the window with our drawing
        glfwSwapBuffers(window);
    }

    // unbind all of our OpenGL objects
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);   

    // delete all of our OpenGL objects
    glDeleteShader(shader);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    // Destroy the window
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}