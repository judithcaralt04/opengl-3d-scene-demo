#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vector>

GLuint gl_createAndBindVAO();
void gl_createAndBindAttribute(const GLfloat data[], int data_size, GLuint shader, const char* attrib, GLuint attrib_size);
void gl_createIndexBuffer(const GLuint* data, int data_size);
void gl_unbindVAO();
void gl_bindVAO(GLuint vao);
GLuint gl_createTextureFromData(int width, int height, const unsigned char* data);
GLuint gl_loadTextureFromFile(const char* path);

// Funciones para crear texturas procedimentales
GLuint gl_createProceduralNormalMap(int width, int height);
GLuint gl_createProceduralSpecularMap(int width, int height, float intensity = 0.5f);
GLuint gl_createProceduralAlphaMap(int width, int height, float alpha = 1.0f);
GLuint gl_createProceduralTexture(int width, int height, float r, float g, float b);
GLuint gl_createTextureRGBA(int width, int height, const unsigned char* data);