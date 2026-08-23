#include "glfunctions.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint gl_createAndBindVAO() {
	GLuint new_vao;
	glGenVertexArrays(1, &new_vao); // glGenVertexArrays requiere una referencia
	glBindVertexArray(new_vao); // Enlazarlo ahora para poder empezar a escribir en él
	return new_vao;
}

void gl_createAndBindAttribute(const GLfloat data[], int data_size, GLuint shader, const char* attrib, GLuint attrib_size) {
	// Variable para almacenar el buffer creado
	GLuint buffer;
	// Crear VBO para vértices
	glGenBuffers(1, &buffer); // glGenBuffers requiere una referencia
	glBindBuffer(GL_ARRAY_BUFFER, buffer); // Enlazar nuestro nuevo buffer
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW); // Llenarlo con nuestros vértices

	// Activar atributo del shader para este buffer
	GLuint vertexLoc = glGetAttribLocation(shader, attrib); // Buscar atributo en el shader
	glEnableVertexAttribArray(vertexLoc); // Habilitarlo
	glVertexAttribPointer(vertexLoc, attrib_size, GL_FLOAT, GL_FALSE, 0, 0); // Especificar ubicación y formato de datos

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Desenlazar buffers
}

void gl_createIndexBuffer(const GLuint* data, int data_size) {
	GLuint buffer;
	// Crear VBO para índices
	glGenBuffers(1, &buffer); // Crear nuevos buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); // Enlazarlos como array de elementos
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW); // Llenar buffer con datos de índices
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Desenlazar buffers
}

void gl_unbindVAO() {
	glBindVertexArray(0); // Desenlazar VAO
}

void gl_bindVAO(GLuint vao) {
	glBindVertexArray(vao);
}

// Crear textura desde datos de imagen
GLuint gl_createTextureFromData(int width, int height, const unsigned char* data) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

// Cargar textura desde archivo usando stb_image
GLuint gl_loadTextureFromFile(const char* path) {
	int width, height, numChannels;
	
	// Voltear verticalmente (las imágenes OpenGL tienen origen en esquina inferior)
	stbi_set_flip_vertically_on_load(true);
	
	// Cargar la imagen
	unsigned char* pixels = stbi_load(path, &width, &height, &numChannels, 0);
	
	if (!pixels) {
		std::cout << "Error: No se ha podido cargar la textura: " << path << std::endl;
		return 0;
	}
	
	// Crear textura
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	// Determinar formato basado en número de canales
	GLenum format = GL_RGB;
	if (numChannels == 1) format = GL_RED;
	else if (numChannels == 3) format = GL_RGB;
	else if (numChannels == 4) format = GL_RGBA;
	
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Liberar memoria de la imagen
	stbi_image_free(pixels);
	
	return textureID;
}

// Crear textura RGBA desde datos
GLuint gl_createTextureRGBA(int width, int height, const unsigned char* data) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

// Crear mapa de normales procedimental
GLuint gl_createProceduralNormalMap(int width, int height) {
	std::vector<unsigned char> data(width * height * 3);
	
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = (y * width + x) * 3;
			
			// Crear un patrón de normal map básico
			// Normal apuntando principalmente hacia arriba (0.5, 0.5, 1.0) en espacio de textura
			float fx = (float)x / width;
			float fy = (float)y / height;
			
			// Añadir variación con sin/cos para crear relieve
			float nx = sin(fx * 3.14159f * 4.0f) * 0.3f + 0.5f;
			float ny = sin(fy * 3.14159f * 4.0f) * 0.3f + 0.5f;
			float nz = 0.8f + sin(fx * 3.14159f * 2.0f) * sin(fy * 3.14159f * 2.0f) * 0.2f;
			
			// Normalizar y convertir a [0, 255]
			float len = sqrt(nx*nx + ny*ny + nz*nz);
			nx /= len; ny /= len; nz /= len;
			
			data[idx] = (unsigned char)((nx + 1.0f) * 0.5f * 255);
			data[idx + 1] = (unsigned char)((ny + 1.0f) * 0.5f * 255);
			data[idx + 2] = (unsigned char)((nz + 1.0f) * 0.5f * 255);
		}
	}
	
	return gl_createTextureFromData(width, height, &data[0]);
}

// Crear mapa especular procedimental
GLuint gl_createProceduralSpecularMap(int width, int height, float intensity) {
	std::vector<unsigned char> data(width * height);
	
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = y * width + x;
			
			float fx = (float)x / width;
			float fy = (float)y / height;
			
			// Crear un patrón de brillo variable
			float spec = intensity + sin(fx * 3.14159f * 6.0f) * sin(fy * 3.14159f * 6.0f) * 0.3f;
			spec = std::max(0.0f, std::min(1.0f, spec));
			
			data[idx] = (unsigned char)(spec * 255);
		}
	}
	
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, &data[0]);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

// Crear mapa alfa procedimental
GLuint gl_createProceduralAlphaMap(int width, int height, float alpha) {
	std::vector<unsigned char> data(width * height);
	
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = y * width + x;
			
			float fx = (float)x / width;
			float fy = (float)y / height;
			
			// Crear un patrón de transparencia (por ejemplo, gradiente circular)
			float dist = sqrt((fx - 0.5f) * (fx - 0.5f) + (fy - 0.5f) * (fy - 0.5f));
			float a = alpha * (1.0f - dist * 1.5f);
			a = std::max(0.0f, std::min(1.0f, a));
			
			data[idx] = (unsigned char)(a * 255);
		}
	}
	
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, &data[0]);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

// Crear textura de color sólido procedimental
GLuint gl_createProceduralTexture(int width, int height, float r, float g, float b) {
	std::vector<unsigned char> data(width * height * 3);
	
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = (y * width + x) * 3;
			
			// Añadir pequeña variación para evitar banding
			float fx = (float)x / width;
			float fy = (float)y / height;
			float noise = (sin(fx * 10.0f) * sin(fy * 10.0f) + 1.0f) * 0.5f * 0.1f;
			
			data[idx] = (unsigned char)((r + noise) * 255);
			data[idx + 1] = (unsigned char)((g + noise) * 255);
			data[idx + 2] = (unsigned char)((b + noise) * 255);
		}
	}
	
	return gl_createTextureFromData(width, height, &data[0]);
}