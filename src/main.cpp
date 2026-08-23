/*  Base code by Alun Evans 2016 LaSalle (aevanss@salleurl.edu) modified by: Conrado Ruiz, Ferran Ruiz 2024*/

// student name: judith.caralt

// Incluir librerías estándar
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

// Incluir librerías de OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM para matrices
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Incluir archivos de código personalizado
#include "glfunctions.h"	// Funciones auxiliares de OpenGL
#include "Shader.h"			// Clase para compilar shaders
#include "SceneObject.h"	// Estructura para objetos de la escena

using namespace std;
using namespace glm;

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// Archivos OBJ disponibles
const char sphereFile[] = "assets/sphere.obj";
const char bunnyFile[] = "assets/bunny.obj";
const char teapotFile[] = "assets/teapot.obj";
const char planeFile[] = "assets/plane.obj";

// Variables globales
int g_ViewportWidth = 1024; 
int g_ViewportHeight = 768; 
double mouse_x, mouse_y;
const vec3 g_backgroundColor(0.1f, 0.1f, 0.15f);

// Shaders
GLuint g_simpleShader = 0;
GLuint g_simpleShader_sky = 0;
GLuint g_shadowShader = 0; // Shader para mapeo de sombras

// VAOs para los diferentes tipos de modelos (al menos 3 formas diferentes)
GLuint g_VaoSphere = 0;
GLuint g_NumTrianglesSphere = 0;

GLuint g_VaoBunny = 0;
GLuint g_NumTrianglesBunny = 0;

GLuint g_VaoTeapot = 0;
GLuint g_NumTrianglesTeapot = 0;

GLuint g_VaoPlane = 0;
GLuint g_NumTrianglesPlane = 0;

GLuint g_VaoPlaneObj = 0;
GLuint g_NumTrianglesPlaneObj = 0;

GLuint g_VaoSky = 0;
GLuint g_NumTrianglesSky = 0;
GLuint g_textureSky = 0;

// Vector de objetos en la escena (al menos 10 instancias)
vector<SceneObject> g_sceneObjects;

// Texturas (al menos 10 distintas)
vector<GLuint> g_textures;

// Variables de cámara FPS
vec3 g_cameraPos = vec3(0.0f, 1.0f, 5.0f);
vec3 g_cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 g_cameraUp = vec3(0.0f, 1.0f, 0.0f);
float g_cameraSpeed = 2.5f;
float g_cameraYaw = -90.0f;
float g_cameraPitch = 0.0f;
bool g_firstMouse = true;
double g_lastX = 0.0;
double g_lastY = 0.0;
float g_mouseSensitivity = 0.1f;

// DeltaTime para movimiento independiente de frames
float g_deltaTime = 0.0f;
float g_lastFrame = 0.0f;

// Variables de iluminación
vec3 g_light_dir = vec3(5.0f, 8.0f, 5.0f);

// Variables para mapeo de sombras
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
GLuint g_depthMapFBO = 0;
GLuint g_depthMap = 0;
bool g_useShadows = true;

// ------------------------------------------------------------------------------------------
// Función para configurar el mapeo de sombras
// ------------------------------------------------------------------------------------------
void setupShadowMapping() {
	// Crear FBO para mapa de profundidad
	glGenFramebuffers(1, &g_depthMapFBO);
	
	// Crear textura de profundidad
	glGenTextures(1, &g_depthMap);
	glBindTexture(GL_TEXTURE_2D, g_depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	// Adjuntar textura de profundidad al FBO
	glBindFramebuffer(GL_FRAMEBUFFER, g_depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cout << "Error: El FBO de mapeo de sombras no está completo" << endl;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	cout << "Mapeo de sombras configurado" << endl;
}

// ------------------------------------------------------------------------------------------
// Función para renderizar la escena desde la perspectiva de la luz (para mapeo de sombras)
// ------------------------------------------------------------------------------------------
void renderShadowMap() {
	if (!g_useShadows) return;
	
	// Calcular matriz de vista y proyección desde la perspectiva de la luz
	mat4 lightProjection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);
	mat4 lightView = lookAt(g_light_dir, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 lightSpaceMatrix = lightProjection * lightView;
	
	// Renderizar al mapa de profundidad
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, g_depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(g_shadowShader);
	
	GLuint lightSpaceMatrixLoc = glGetUniformLocation(g_shadowShader, "u_lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	
	// Renderizar todos los objetos opacos
	for (const SceneObject& obj : g_sceneObjects) {
		if (obj.isTransparent) continue; // Saltar objetos transparentes
		
		mat4 model(1.0f);
		model = translate(model, obj.position);
		model = rotate(model, obj.rotationX, vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, obj.rotationY, vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, obj.rotationZ, vec3(0.0f, 0.0f, 1.0f));
		model = scale(model, obj.scale);
		
		GLuint modelLoc = glGetUniformLocation(g_shadowShader, "u_model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		gl_bindVAO(obj.vao);
		glDrawElements(GL_TRIANGLES, 3 * obj.numTriangles, GL_UNSIGNED_INT, 0);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, g_ViewportWidth, g_ViewportHeight);
}

// ------------------------------------------------------------------------------------------
// Función para crear un modelo de avión simple 
// ------------------------------------------------------------------------------------------
GLuint createPlaneVAO(GLuint shader, GLuint& numTriangles) {
	// Crear geometría de un avión simple (forma de flecha/triángulo)
	// Fuselaje: cuerpo principal
	// Alas: dos triángulos a los lados
	// Cola: pequeño triángulo trasero
	
	vector<float> positions;
	vector<float> normals;
	vector<float> texcoords;
	vector<unsigned int> indices;
	
	// Avión simple: forma de flecha con alas
	// Fuselaje (cuerpo principal)
	positions.insert(positions.end(), {0.0f, 0.0f, 1.0f}); // 0: Punta delantera
	positions.insert(positions.end(), {-0.2f, 0.0f, -0.5f}); // 1: Ala izquierda delantera
	positions.insert(positions.end(), {0.2f, 0.0f, -0.5f}); // 2: Ala derecha delantera
	positions.insert(positions.end(), {-0.15f, 0.0f, -1.0f}); // 3: Ala izquierda trasera
	positions.insert(positions.end(), {0.15f, 0.0f, -1.0f}); // 4: Ala derecha trasera
	positions.insert(positions.end(), {0.0f, 0.0f, -1.2f}); // 5: Cola trasera
	
	// Parte superior del fuselaje
	positions.insert(positions.end(), {0.0f, 0.15f, 0.5f}); // 6: Superior delantera
	positions.insert(positions.end(), {0.0f, 0.1f, -1.0f}); // 7: Superior trasera
	
	// Cola vertical
	positions.insert(positions.end(), {0.0f, 0.1f, -1.0f}); // 8: Base cola (mismo que 7)
	positions.insert(positions.end(), {0.0f, 0.6f, -1.3f}); // 9: Punta cola vertical
	positions.insert(positions.end(), {-0.1f, 0.1f, -1.0f}); // 10: Cola izquierda
	positions.insert(positions.end(), {0.1f, 0.1f, -1.0f}); // 11: Cola derecha
	
	// Generar normales (simplificado - todas apuntando hacia arriba o hacia afuera)
	for (size_t i = 0; i < positions.size() / 3; i++) {
		float x = positions[i * 3];
		float y = positions[i * 3 + 1];
		float z = positions[i * 3 + 2];
		
		// Calcular normal basada en la posición
		vec3 pos(x, y, z);
		vec3 normal;
		if (y > 0.1f) {
			normal = vec3(0.0f, 1.0f, 0.0f); // Parte superior
	} else {
			normal = normalize(vec3(x * 0.3f, 0.5f, z * 0.3f)); // Lados
		}
		normals.push_back(normal.x);
		normals.push_back(normal.y);
		normals.push_back(normal.z);
		
		// Coordenadas de textura simples
		texcoords.push_back((x + 1.5f) / 3.0f);
		texcoords.push_back((z + 1.5f) / 3.0f);
	}
	
	// Crear índices para las caras
	// Fuselaje inferior (cara principal)
	indices.insert(indices.end(), {0, 1, 2}); // Punta a alas delanteras
	indices.insert(indices.end(), {1, 3, 2}); // Ala izquierda
	indices.insert(indices.end(), {2, 3, 4}); // Ala derecha
	indices.insert(indices.end(), {3, 5, 4}); // Hacia la cola
	
	// Fuselaje superior
	indices.insert(indices.end(), {0, 6, 1}); // Lado izquierdo superior
	indices.insert(indices.end(), {0, 2, 6}); // Lado derecho superior
	indices.insert(indices.end(), {6, 7, 1}); // Superior izquierda
	indices.insert(indices.end(), {6, 2, 7}); // Superior derecha
	indices.insert(indices.end(), {1, 7, 3}); // Lado izquierdo
	indices.insert(indices.end(), {2, 4, 7}); // Lado derecho
	indices.insert(indices.end(), {3, 7, 5}); // Trasera izquierda
	indices.insert(indices.end(), {4, 5, 7}); // Trasera derecha
	
	// Cola vertical
	indices.insert(indices.end(), {8, 10, 9}); // Cara izquierda cola
	indices.insert(indices.end(), {8, 9, 11}); // Cara derecha cola
	indices.insert(indices.end(), {10, 11, 9}); // Trasera cola
	
	// Crear VAO
	GLuint vao = gl_createAndBindVAO();
	
	// Posiciones
	gl_createAndBindAttribute(&(positions[0]),
		sizeof(float) * positions.size(),
		shader, "a_vertex", 3);
	
	// Coordenadas de textura
	gl_createAndBindAttribute(&(texcoords[0]),
		sizeof(float) * texcoords.size(),
		shader, "a_texcoord", 2);
	
	// Normales
	gl_createAndBindAttribute(&(normals[0]),
		sizeof(float) * normals.size(),
		shader, "a_normal", 3);
	
	// Índices
	gl_createIndexBuffer(&(indices[0]),
		sizeof(unsigned int) * indices.size());

    gl_unbindVAO();
	numTriangles = indices.size() / 3;
	
	cout << "Modelo de avión creado con " << numTriangles << " triángulos" << endl;
	
	return vao;
    }

// ------------------------------------------------------------------------------------------
// Función auxiliar para cargar un modelo OBJ y crear su VAO
// ------------------------------------------------------------------------------------------
GLuint loadModelVAO(const char* filename, GLuint shader, GLuint& numTriangles) {
	vector<tinyobj::shape_t> shapes;
	bool ret = tinyobj::LoadObj(shapes, filename);
	if (!ret) {
		cout << "Error: No se pudo cargar " << filename << endl;
		return 0;
	}
	cout << "OBJ File: " << filename << " cargado exitosamente" << endl;

	GLuint vao = gl_createAndBindVAO();
	
	// Posiciones de vértices
	gl_createAndBindAttribute(&(shapes[0].mesh.positions[0]),
		sizeof(float) * shapes[0].mesh.positions.size(),
		shader, "a_vertex", 3);
    
	// Coordenadas de textura
	vector<float> texcoords;
	if (shapes[0].mesh.texcoords.size() > 0) {
		texcoords = shapes[0].mesh.texcoords;
    } else {
		// Generar coordenadas de textura por defecto
		texcoords.resize(shapes[0].mesh.positions.size() / 3 * 2);
		for (size_t i = 0; i < shapes[0].mesh.positions.size() / 3; i++) {
			float x = shapes[0].mesh.positions[i * 3];
			float y = shapes[0].mesh.positions[i * 3 + 1];
			float z = shapes[0].mesh.positions[i * 3 + 2];
			// Mapeo esférico 
			texcoords[i * 2] = atan2(x, z) / (2.0f * 3.14159f) + 0.5f;
			texcoords[i * 2 + 1] = acos(y) / 3.14159f;
        }
    }
    
	gl_createAndBindAttribute(&(texcoords[0]),
		sizeof(float) * texcoords.size(),
		shader, "a_texcoord", 2);
	
	// Normales
	if (shapes[0].mesh.normals.size() > 0) {
		gl_createAndBindAttribute(&(shapes[0].mesh.normals[0]),
			shapes[0].mesh.normals.size() * sizeof(float),
			shader, "a_normal", 3);
	} else {
		// Generar normales desde posiciones
		vector<float> normals;
		normals.resize(shapes[0].mesh.positions.size());
		for (size_t i = 0; i < shapes[0].mesh.positions.size() / 3; i++) {
			vec3 pos = vec3(
				shapes[0].mesh.positions[i * 3],
				shapes[0].mesh.positions[i * 3 + 1],
				shapes[0].mesh.positions[i * 3 + 2]
			);
			vec3 normal = normalize(pos);
			normals[i * 3] = normal.x;
			normals[i * 3 + 1] = normal.y;
			normals[i * 3 + 2] = normal.z;
		}
		gl_createAndBindAttribute(&(normals[0]),
			normals.size() * sizeof(float),
			shader, "a_normal", 3);
	}
    
	gl_createIndexBuffer(&(shapes[0].mesh.indices[0]),
		sizeof(unsigned int) * shapes[0].mesh.indices.size());
	
    gl_unbindVAO();
	numTriangles = shapes[0].mesh.indices.size() / 3;
	
	return vao;
}

// ------------------------------------------------------------------------------------------
// Función auxiliar para calcular distancia entre dos objetos
// ------------------------------------------------------------------------------------------
float distanceBetweenObjects(const SceneObject& obj1, const SceneObject& obj2) {
	vec3 diff = obj1.position - obj2.position;
	return length(diff);
}

// ------------------------------------------------------------------------------------------
// Función para verificar y evitar colisiones simples
// ------------------------------------------------------------------------------------------
void avoidCollisions() {
	const float minDistance = 2.0f; // Distancia mínima entre objetos
	
	for (size_t i = 0; i < g_sceneObjects.size(); i++) {
		for (size_t j = i + 1; j < g_sceneObjects.size(); j++) {
			float dist = distanceBetweenObjects(g_sceneObjects[i], g_sceneObjects[j]);
			
			// Si los objetos están muy cerca, ajustar posición
			if (dist < minDistance && dist > 0.01f) {
				vec3 direction = normalize(g_sceneObjects[i].position - g_sceneObjects[j].position);
				float pushDistance = (minDistance - dist) * 0.5f;
				
				// Empujar objetos en direcciones opuestas
				g_sceneObjects[i].position += direction * pushDistance;
				g_sceneObjects[j].position -= direction * pushDistance;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------
// Función para actualizar animaciones de objetos
// ------------------------------------------------------------------------------------------
void updateAnimations(float deltaTime) {
	for (size_t i = 0; i < g_sceneObjects.size(); i++) {
		SceneObject& obj = g_sceneObjects[i];
		obj.animationTime += deltaTime * obj.animationSpeed;
	
		// Guardar posición anterior
		vec3 oldPosition = obj.position;
		
		switch (obj.animationType) {
			case 1: // Órbita circular (o vuelo en círculo para avión)
			{
				float angle = obj.animationTime;
				obj.position.x = obj.animationCenter.x + obj.animationRadius * cos(angle);
				obj.position.z = obj.animationCenter.z + obj.animationRadius * sin(angle);
				
				// Para aviones, rotar hacia la dirección del movimiento
				if (obj.vao == g_VaoPlane || obj.vao == g_VaoPlaneObj) {
					// Rotar el avión para que apunte en la dirección del movimiento
					obj.rotationY = angle + 3.14159f / 2.0f; // Ajustar orientación (90 grados)
					// Inclinar ligeramente en las curvas (banking)
					obj.rotationZ = sin(angle) * 0.4f;
					// Mantener el avión con ligera inclinación hacia adelante
					obj.rotationX = -0.2f; // Inclinación hacia adelante (pitch)
				} else {
					obj.rotationY = angle;
				}
				break;
			}
			case 2: // Flotación vertical (solo en Y, no se mueve en XZ)
			{
				obj.position.y = obj.basePosition.y + sin(obj.animationTime) * 0.5f;
				obj.position.x = obj.basePosition.x; // Mantener posición X original
				obj.position.z = obj.basePosition.z; // Mantener posición Z original
				obj.rotationY += deltaTime * 0.5f;
				break;
			}
			case 3: // Espiral (limitada para evitar cruces)
			{
				float angle = obj.animationTime;
				// Limitar el radio máximo para evitar que crezca demasiado
				float maxRadius = obj.animationRadius + 2.0f;
				float radius = (obj.animationRadius + obj.animationTime * 0.3f < maxRadius) ? 
					obj.animationRadius + obj.animationTime * 0.3f : maxRadius;
				obj.position.x = obj.animationCenter.x + radius * cos(angle);
				obj.position.z = obj.animationCenter.z + radius * sin(angle);
				obj.position.y = obj.basePosition.y + sin(obj.animationTime * 2.0f) * 0.3f;
				obj.rotationY = angle * 2.0f;
				obj.rotationX = sin(obj.animationTime) * 0.3f;
				break;
			}
			case 4: // Movimiento de péndulo (solo en plano XZ o XY)
			{
				float angle = sin(obj.animationTime) * 1.5f;
				obj.position.x = obj.basePosition.x + sin(angle) * obj.animationRadius;
				obj.position.y = obj.basePosition.y + cos(angle) * obj.animationRadius * 0.5f;
				obj.position.z = obj.basePosition.z; // Mantener Z fija para péndulo vertical
				obj.rotationZ = angle;
				break;
			}
			case 5: // Órbita elíptica con rotación
			{
				float angle = obj.animationTime;
				obj.position.x = obj.animationCenter.x + obj.animationRadius * cos(angle);
				obj.position.z = obj.animationCenter.z + obj.animationRadius * 0.6f * sin(angle);
				obj.position.y = obj.basePosition.y + sin(angle * 2.0f) * 0.4f;
				obj.rotationY = angle * 3.0f;
				obj.rotationX = cos(angle) * 0.5f;
				break;
			}
			default:
				// Rotación simple si no hay animación de posición
				obj.rotationY += deltaTime * 0.5f;
				break;
		}
	}
	
	// Verificar y evitar colisiones después de actualizar todas las animaciones
	avoidCollisions();
}

// ------------------------------------------------------------------------------------------
// Función para cargar todos los recursos
// ------------------------------------------------------------------------------------------
void load()
{
	// Cargar shaders
	Shader simpleShader("src/shader.vert", "src/shader.frag");
	g_simpleShader = simpleShader.program;
	
	Shader simpleShader_sky("src/shader_sky.vert", "src/shader_sky.frag");
	g_simpleShader_sky = simpleShader_sky.program;
	
	// Cargar shader para shadow mapping
	Shader shadowShader("src/shader_shadow.vert", "src/shader_shadow.frag");
	g_shadowShader = shadowShader.program;
	
	// Configurar shadow mapping
	setupShadowMapping();
	
	// Cargar modelos (3 formas diferentes)
	g_VaoSphere = loadModelVAO(sphereFile, g_simpleShader, g_NumTrianglesSphere);
	g_VaoBunny = loadModelVAO(bunnyFile, g_simpleShader, g_NumTrianglesBunny);
	g_VaoTeapot = loadModelVAO(teapotFile, g_simpleShader, g_NumTrianglesTeapot);
	
	// Crear modelo de avión procedimentalmente
	g_VaoPlane = createPlaneVAO(g_simpleShader, g_NumTrianglesPlane);
	
	// Cargar plane
	g_VaoPlaneObj = loadModelVAO(planeFile, g_simpleShader, g_NumTrianglesPlaneObj);
	
	// Cargar skybox
	g_VaoSky = loadModelVAO(sphereFile, g_simpleShader_sky, g_NumTrianglesSky);
	g_textureSky = gl_loadTextureFromFile("assets/milkyway.bmp");
	if (g_textureSky == 0) {
		cout << "Warning: No se pudo cargar la textura del skybox (milkyway.bmp)" << endl;
	} else {
		// Configurar parámetros de textura para skybox
		glBindTexture(GL_TEXTURE_2D, g_textureSky);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		cout << "Skybox texture cargada exitosamente" << endl;
	}
	
	// Crear texturas (al menos 10 distintas)
	// 1. Textura de Earth
	g_textures.push_back(gl_loadTextureFromFile("assets/earth.bmp"));
	
	// 2. Textura de Moon
	g_textures.push_back(gl_loadTextureFromFile("assets/moon.bmp"));
	
	// 3. Textura de Marble
	g_textures.push_back(gl_loadTextureFromFile("assets/marble.bmp"));
	
	// Textura de Barbie
	GLuint textureBarbie = gl_loadTextureFromFile("assets/Barbiebox.png");
	if (textureBarbie == 0) {
		cout << "Warning: No se pudo cargar Barbiebox.png, usando textura procedimental" << endl;
		// Usar una textura procedimental como fallback (rosa Barbie)
		textureBarbie = gl_createProceduralTexture(256, 256, 1.0f, 0.4f, 0.7f);
	} else {
		cout << "Textura Barbie cargada exitosamente" << endl;
	}
	g_textures.push_back(textureBarbie);
	
	// 4-10. Texturas procedimentales
	g_textures.push_back(gl_createProceduralTexture(256, 256, 0.8f, 0.2f, 0.2f)); // Rojo
	g_textures.push_back(gl_createProceduralTexture(256, 256, 0.2f, 0.8f, 0.2f)); // Verde
	g_textures.push_back(gl_createProceduralTexture(256, 256, 0.2f, 0.2f, 0.8f)); // Azul
	g_textures.push_back(gl_createProceduralTexture(256, 256, 0.8f, 0.8f, 0.2f)); // Amarillo
	g_textures.push_back(gl_createProceduralTexture(256, 256, 0.8f, 0.2f, 0.8f)); // Magenta
	g_textures.push_back(gl_createProceduralTexture(256, 256, 0.2f, 0.8f, 0.8f)); // Cyan
	g_textures.push_back(gl_createProceduralTexture(256, 256, 0.5f, 0.5f, 0.5f)); // Gris
	
	// Crear mapas de normales procedimentales
	GLuint normalMap1 = gl_createProceduralNormalMap(256, 256);
	GLuint normalMap2 = gl_createProceduralNormalMap(256, 256);
	
	// Crear mapas especulares procedimentales
	GLuint specularMap1 = gl_createProceduralSpecularMap(256, 256, 0.7f);
	GLuint specularMap2 = gl_createProceduralSpecularMap(256, 256, 0.3f);
	GLuint specularMap3 = gl_createProceduralSpecularMap(256, 256, 0.9f);
	
	// Crear mapas alfa procedimentales (para transparencia)
	GLuint alphaMap1 = gl_createProceduralAlphaMap(256, 256, 0.7f); // Semitransparente
	GLuint alphaMap2 = gl_createProceduralAlphaMap(256, 256, 0.5f); // Más transparente
	
	// Crear objetos de la escena (al menos 10 instancias)
	SceneObject obj;
	
	// Objeto 1: Esfera con textura Earth, normal map y specular map
	obj.vao = g_VaoSphere;
	obj.numTriangles = g_NumTrianglesSphere;
	obj.textureDiffuse = g_textures[0]; // Earth
	obj.textureNormal = normalMap1;
	obj.textureSpecular = specularMap1;
	obj.hasNormalMap = true;
	obj.hasSpecularMap = true;
	obj.position = vec3(-3.0f, 0.0f, -2.0f);
	obj.scale = vec3(0.8f);
	obj.animationType = 0; // Solo rotación
	obj.shininess = 32.0f;
	g_sceneObjects.push_back(obj);
	
	// Objeto 2: Esfera con textura Moon (órbita separada en plano diferente)
	obj.textureDiffuse = g_textures[1]; // Moon
	obj.textureNormal = 0;
	obj.textureSpecular = specularMap2;
	obj.hasNormalMap = false;
	obj.hasSpecularMap = true;
	obj.position = vec3(3.0f, 0.0f, -5.0f);
	obj.animationType = 1; // Órbita
	obj.animationCenter = vec3(0.0f, 0.0f, -5.0f); // Centro separado
	obj.animationRadius = 2.5f; // Radio más pequeño
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 3: Bunny con textura roja y normal map
	obj.vao = g_VaoBunny;
	obj.numTriangles = g_NumTrianglesBunny;
	obj.textureDiffuse = g_textures[3]; // Rojo
	obj.textureNormal = normalMap2;
	obj.textureSpecular = specularMap3;
	obj.hasNormalMap = true;
	obj.hasSpecularMap = true;
	obj.position = vec3(-2.0f, 0.5f, 2.0f);
	obj.scale = vec3(1.5f);
	obj.animationType = 2; // Flotación
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 4: Bunny con textura verde (espiral en altura diferente)
	obj.textureDiffuse = g_textures[4]; // Verde
	obj.textureNormal = normalMap1;
	obj.textureSpecular = specularMap1;
	obj.position = vec3(2.0f, 0.5f, 2.0f);
	obj.animationType = 3; // Espiral
	obj.animationCenter = vec3(0.0f, 0.5f, 2.0f); // Mismo plano pero diferente altura
	obj.animationRadius = 1.5f; // Radio inicial más pequeño
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 5: Teapot con textura azul y alpha map (semitransparente)
	obj.vao = g_VaoTeapot;
	obj.numTriangles = g_NumTrianglesTeapot;
	obj.textureDiffuse = g_textures[5]; // Azul
	obj.textureNormal = normalMap2;
	obj.textureSpecular = specularMap2;
	obj.textureAlpha = alphaMap1;
	obj.hasNormalMap = true;
	obj.hasSpecularMap = true;
	obj.hasAlphaMap = true;
	obj.isTransparent = true;
	obj.position = vec3(0.0f, 1.0f, 0.0f);
	obj.scale = vec3(0.6f);
	obj.animationType = 4; // Péndulo
	obj.basePosition = obj.position;
	obj.animationRadius = 2.0f;
	g_sceneObjects.push_back(obj);
	
	// Objeto 6: Teapot con textura amarilla (órbita en plano superior)
	obj.textureDiffuse = g_textures[6]; // Amarillo
	obj.textureNormal = normalMap1;
	obj.textureSpecular = specularMap3;
	obj.textureAlpha = 0;
	obj.hasAlphaMap = false;
	obj.isTransparent = false;
	obj.position = vec3(-4.0f, 2.0f, 0.0f); // Altura diferente
	obj.scale = vec3(0.5f);
	obj.animationType = 5; // Órbita elíptica
	obj.animationCenter = vec3(0.0f, 2.0f, 0.0f); // Centro en altura diferente
	obj.animationRadius = 3.5f; // Radio ajustado
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 7: Esfera con textura magenta (órbita en plano inferior)
	obj.vao = g_VaoSphere;
	obj.numTriangles = g_NumTrianglesSphere;
	obj.textureDiffuse = g_textures[7]; // Magenta
	obj.textureNormal = normalMap2;
	obj.textureSpecular = specularMap1;
	obj.hasNormalMap = true;
	obj.hasSpecularMap = true;
	obj.position = vec3(4.0f, -1.0f, 0.0f); // Altura negativa
	obj.scale = vec3(0.7f);
	obj.animationType = 1; // Órbita
	obj.animationCenter = vec3(0.0f, -1.0f, 0.0f); // Centro en altura diferente
	obj.animationRadius = 3.5f; // Radio ajustado
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 8: Avión con textura cyan (vuelo en círculo)
	obj.vao = g_VaoPlane;
	obj.numTriangles = g_NumTrianglesPlane;
	obj.textureDiffuse = g_textures[8]; // Cyan
	obj.textureNormal = normalMap1;
	obj.textureSpecular = specularMap2;
	obj.hasNormalMap = true;
	obj.hasSpecularMap = true;
	obj.position = vec3(0.0f, 3.0f, -4.0f); // Posición inicial alta
	obj.scale = vec3(0.8f);
	obj.animationType = 1; // Órbita circular (vuelo en círculo)
	obj.animationCenter = vec3(0.0f, 3.0f, -4.0f);
	obj.animationRadius = 5.0f; // Radio grande para vuelo amplio
	obj.animationSpeed = 0.4f; // Velocidad de vuelo
	obj.basePosition = obj.position;
	obj.shininess = 64.0f; // Avión más brillante
	g_sceneObjects.push_back(obj);
	
	// Objeto 9: Teapot con textura gris y alpha map muy transparente (espiral pequeña)
	obj.vao = g_VaoTeapot;
	obj.numTriangles = g_NumTrianglesTeapot;
	obj.textureDiffuse = g_textures[9]; // Gris
	obj.textureNormal = normalMap2;
	obj.textureSpecular = specularMap3;
	obj.textureAlpha = alphaMap2;
	obj.hasAlphaMap = true;
	obj.isTransparent = true;
	obj.position = vec3(-1.5f, 1.5f, 1.0f);
	obj.scale = vec3(0.4f);
	obj.animationType = 3; // Espiral
	obj.animationCenter = vec3(0.0f, 1.5f, 1.0f);
	obj.animationRadius = 1.0f; // Radio inicial pequeño para espiral pequeña
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 10: Esfera con textura marble
	obj.vao = g_VaoSphere;
	obj.numTriangles = g_NumTrianglesSphere;
	obj.textureDiffuse = g_textures[2]; // Marble
	obj.textureNormal = normalMap1;
	obj.textureSpecular = specularMap1;
	obj.textureAlpha = 0;
	obj.hasAlphaMap = false;
	obj.isTransparent = false;
	obj.position = vec3(1.5f, 1.5f, 1.0f);
	obj.scale = vec3(0.6f);
	obj.animationType = 4; // Péndulo
	obj.basePosition = obj.position;
	obj.animationRadius = 1.5f;
	g_sceneObjects.push_back(obj);
	
	// Objeto 11: Bunny adicional con textura roja (más instancias)
	obj.vao = g_VaoBunny;
	obj.numTriangles = g_NumTrianglesBunny;
	obj.textureDiffuse = g_textures[3];
	obj.textureNormal = normalMap2;
	obj.textureSpecular = specularMap2;
	obj.hasNormalMap = true;
	obj.hasSpecularMap = true;
	obj.textureAlpha = 0;
	obj.hasAlphaMap = false;
	obj.isTransparent = false;
	obj.position = vec3(0.0f, 0.0f, -6.0f);
	obj.scale = vec3(1.0f);
	obj.animationType = 5; // Órbita elíptica
	obj.animationCenter = vec3(0.0f, 0.0f, -6.0f);
	obj.animationRadius = 3.0f;
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 12: Esfera adicional con textura verde
	obj.vao = g_VaoSphere;
	obj.numTriangles = g_NumTrianglesSphere;
	obj.textureDiffuse = g_textures[4];
	obj.textureNormal = normalMap1;
	obj.textureSpecular = specularMap3;
	obj.position = vec3(-2.0f, 2.0f, -3.0f);
	obj.scale = vec3(0.5f);
	obj.animationType = 1; // Órbita
	obj.animationCenter = vec3(-2.0f, 2.0f, -3.0f);
	obj.animationRadius = 2.0f;
	obj.basePosition = obj.position;
	g_sceneObjects.push_back(obj);
	
	// Objeto 13: Avión (plane.obj) con textura de Barbie
	obj.vao = g_VaoPlaneObj;
	obj.numTriangles = g_NumTrianglesPlaneObj;
	obj.textureDiffuse = g_textures[3]; // Textura de Barbie (índice 3 después de earth, moon, marble)
	obj.textureNormal = normalMap1;
	obj.textureSpecular = specularMap2;
	obj.hasNormalMap = true;
	obj.hasSpecularMap = true;
	obj.textureAlpha = 0;
	obj.hasAlphaMap = false;
	obj.isTransparent = false;
	obj.position = vec3(0.0f, 4.0f, -8.0f); // Posición inicial alta y atrás
	obj.scale = vec3(1.5f); // Escala del avión
	obj.rotationX = 0.0f;
	obj.rotationY = 0.0f;
	obj.rotationZ = 0.0f;
	obj.animationType = 1; // Órbita circular (vuelo en círculo)
	obj.animationCenter = vec3(0.0f, 4.0f, -8.0f);
	obj.animationRadius = 6.0f; // Radio grande para vuelo amplio
	obj.animationSpeed = 0.3f; // Velocidad de vuelo
	obj.basePosition = obj.position;
	obj.shininess = 64.0f; // Avión más brillante
	g_sceneObjects.push_back(obj);
	
	cout << "Escena cargada con " << g_sceneObjects.size() << " objetos" << endl;
}

// ------------------------------------------------------------------------------------------
// Función para renderizar la escena
// ------------------------------------------------------------------------------------------
void draw()
{
	// Limpiar pantalla
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Calcular matriz de vista (cámara FPS)
	mat4 view = lookAt(g_cameraPos, g_cameraPos + g_cameraFront, g_cameraUp);
	
	// Calcular matriz de proyección (perspectiva)
	mat4 projection = perspective(radians(45.0f), 
		(float)g_ViewportWidth / (float)g_ViewportHeight, 0.1f, 100.0f);
	
	// Actualizar animaciones
	updateAnimations(g_deltaTime);
	
	// ========== RENDERIZAR SHADOW MAP PRIMERO ==========
	renderShadowMap();
	
	// ========== DIBUJAR SKYBOX PRIMERO ==========
	// Guardar estado de depth test
	GLboolean depthTestEnabled;
	glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
	
	// Configurar para renderizar skybox
	glDepthFunc(GL_LEQUAL); // Permitir que el skybox se renderice en la profundidad más lejana
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT); // Cull front faces para skybox (estamos dentro de la esfera)
	
	glUseProgram(g_simpleShader_sky);
	
	// Skybox debe ser muy grande y siempre centrado en la cámara
	mat4 model_sky = translate(mat4(1.0f), g_cameraPos);
	model_sky = scale(model_sky, vec3(50.0f)); // Hacer el skybox muy grande
	
	// Remover traslación de la vista para que el skybox siempre esté centrado en la cámara
	mat4 view_sky = mat4(mat3(view));
	
	GLuint model_loc_sky = glGetUniformLocation(g_simpleShader_sky, "u_model");
	GLuint view_loc_sky = glGetUniformLocation(g_simpleShader_sky, "u_view");
	GLuint projection_loc_sky = glGetUniformLocation(g_simpleShader_sky, "u_projection");
	
	if (model_loc_sky != -1) glUniformMatrix4fv(model_loc_sky, 1, GL_FALSE, glm::value_ptr(model_sky));
	if (view_loc_sky != -1) glUniformMatrix4fv(view_loc_sky, 1, GL_FALSE, glm::value_ptr(view_sky));
	if (projection_loc_sky != -1) glUniformMatrix4fv(projection_loc_sky, 1, GL_FALSE, glm::value_ptr(projection));
	
	GLuint texture_loc_sky = glGetUniformLocation(g_simpleShader_sky, "u_texture");
	if (texture_loc_sky != -1) {
	glUniform1i(texture_loc_sky, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_textureSky);
	}
	
	gl_bindVAO(g_VaoSky);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTrianglesSky, GL_UNSIGNED_INT, 0);
	
	// Restaurar estado de depth test
	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);
	if (!depthTestEnabled) {
		glDisable(GL_DEPTH_TEST);
	}
	
	// ========== DIBUJAR OBJETOS DE LA ESCENA ==========
	glUseProgram(g_simpleShader);
	
	// Configurar matrices de vista y proyección
	GLuint view_loc = glGetUniformLocation(g_simpleShader, "u_view");
	GLuint projection_loc = glGetUniformLocation(g_simpleShader, "u_projection");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
	
	// Configurar iluminación (Blinn-Phong)
	GLuint light_loc = glGetUniformLocation(g_simpleShader, "u_light_dir");
	GLuint cam_pos_loc = glGetUniformLocation(g_simpleShader, "u_cam_pos");
	GLuint ambient_loc = glGetUniformLocation(g_simpleShader, "u_ambient");
	GLuint diffuse_loc = glGetUniformLocation(g_simpleShader, "u_diffuse");
	GLuint specular_loc = glGetUniformLocation(g_simpleShader, "u_specular");
	GLuint shininess_loc = glGetUniformLocation(g_simpleShader, "u_shininess");
	
	glUniform3f(light_loc, g_light_dir.x, g_light_dir.y, g_light_dir.z);
	glUniform3f(cam_pos_loc, g_cameraPos.x, g_cameraPos.y, g_cameraPos.z);
	glUniform3f(ambient_loc, 0.15f, 0.15f, 0.15f);
	glUniform3f(diffuse_loc, 1.0f, 1.0f, 1.0f);
	glUniform3f(specular_loc, 1.0f, 1.0f, 1.0f);
	glUniform1f(shininess_loc, 32.0f);
	
	// Uniforms para texturas
	GLuint textureLoc = glGetUniformLocation(g_simpleShader, "u_texture");
	GLuint textureNormalLoc = glGetUniformLocation(g_simpleShader, "u_textureNormal");
	GLuint textureSpecularLoc = glGetUniformLocation(g_simpleShader, "u_textureSpecular");
	GLuint textureAlphaLoc = glGetUniformLocation(g_simpleShader, "u_textureAlpha");
	GLuint useTextureLoc = glGetUniformLocation(g_simpleShader, "u_useTexture");
	GLuint useNormalMapLoc = glGetUniformLocation(g_simpleShader, "u_useNormalMap");
	GLuint useSpecularMapLoc = glGetUniformLocation(g_simpleShader, "u_useSpecularMap");
	GLuint useAlphaMapLoc = glGetUniformLocation(g_simpleShader, "u_useAlphaMap");
	
	// Configurar mapeo de sombras
	mat4 lightProjection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);
	mat4 lightView = lookAt(g_light_dir, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 lightSpaceMatrix = lightProjection * lightView;
	
	GLuint shadowMapLoc = glGetUniformLocation(g_simpleShader, "u_shadowMap");
	GLuint useShadowsLoc = glGetUniformLocation(g_simpleShader, "u_useShadows");
	GLuint lightSpaceMatrixLoc = glGetUniformLocation(g_simpleShader, "u_lightSpaceMatrix");
	
	glUniform1i(shadowMapLoc, 4);
	glUniform1i(useShadowsLoc, g_useShadows ? 1 : 0);
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	
	// Enlazar textura del mapa de sombras
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depthMap);
	
	// Separar objetos opacos y transparentes para renderizado correcto
	vector<size_t> opaqueObjects;
	vector<size_t> transparentObjects;
	
	for (size_t i = 0; i < g_sceneObjects.size(); i++) {
		if (g_sceneObjects[i].isTransparent) {
			transparentObjects.push_back(i);
		} else {
			opaqueObjects.push_back(i);
		}
	}
	
	// Renderizar objetos opacos primero
	for (size_t idx : opaqueObjects) {
		const SceneObject& obj = g_sceneObjects[idx];
		
		// Calcular matriz de modelo
		mat4 model(1.0f);
		model = translate(model, obj.position);
		model = rotate(model, obj.rotationX, vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, obj.rotationY, vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, obj.rotationZ, vec3(0.0f, 0.0f, 1.0f));
		model = scale(model, obj.scale);
	
		// Calcular matriz normal
		mat3 normalMatrix = mat3(transpose(inverse(model)));
	
	GLuint model_loc = glGetUniformLocation(g_simpleShader, "u_model");
	GLuint normal_matrix_loc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
	
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(normal_matrix_loc, 1, GL_FALSE, glm::value_ptr(mat4(normalMatrix)));
	
		// Configurar texturas
		glUniform1i(useTextureLoc, 1);
		glUniform1i(useNormalMapLoc, obj.hasNormalMap ? 1 : 0);
		glUniform1i(useSpecularMapLoc, obj.hasSpecularMap ? 1 : 0);
		glUniform1i(useAlphaMapLoc, 0);
		
		// Enlazar texturas
	glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj.textureDiffuse);
	glUniform1i(textureLoc, 0);
	
		if (obj.hasNormalMap) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, obj.textureNormal);
			glUniform1i(textureNormalLoc, 1);
		}
		
		if (obj.hasSpecularMap) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, obj.textureSpecular);
			glUniform1i(textureSpecularLoc, 2);
		}
		
		// Configurar shininess específico del objeto
		glUniform1f(shininess_loc, obj.shininess);
		
		// Dibujar objeto
		gl_bindVAO(obj.vao);
		glDrawElements(GL_TRIANGLES, 3 * obj.numTriangles, GL_UNSIGNED_INT, 0);
	}
	
	// Renderizar objetos transparentes después (con alpha blending)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE); // No escribir en depth buffer para objetos transparentes
	
	for (size_t idx : transparentObjects) {
		const SceneObject& obj = g_sceneObjects[idx];
		
		// Calcular matriz de modelo
		mat4 model(1.0f);
		model = translate(model, obj.position);
		model = rotate(model, obj.rotationX, vec3(1.0f, 0.0f, 0.0f));
		model = rotate(model, obj.rotationY, vec3(0.0f, 1.0f, 0.0f));
		model = rotate(model, obj.rotationZ, vec3(0.0f, 0.0f, 1.0f));
		model = scale(model, obj.scale);
	
		// Calcular matriz normal
		mat3 normalMatrix = mat3(transpose(inverse(model)));
		
		GLuint model_loc = glGetUniformLocation(g_simpleShader, "u_model");
		GLuint normal_matrix_loc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
	
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(normal_matrix_loc, 1, GL_FALSE, glm::value_ptr(mat4(normalMatrix)));
	
		// Configurar texturas
		glUniform1i(useTextureLoc, 1);
		glUniform1i(useNormalMapLoc, obj.hasNormalMap ? 1 : 0);
		glUniform1i(useSpecularMapLoc, obj.hasSpecularMap ? 1 : 0);
		glUniform1i(useAlphaMapLoc, obj.hasAlphaMap ? 1 : 0);
		
		// Enlazar texturas
	glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj.textureDiffuse);
	glUniform1i(textureLoc, 0);
	
		if (obj.hasNormalMap) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, obj.textureNormal);
			glUniform1i(textureNormalLoc, 1);
		}
		
		if (obj.hasSpecularMap) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, obj.textureSpecular);
			glUniform1i(textureSpecularLoc, 2);
		}
		
		if (obj.hasAlphaMap) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, obj.textureAlpha);
			glUniform1i(textureAlphaLoc, 3);
		}
		
		// Configurar shininess específico del objeto
		glUniform1f(shininess_loc, obj.shininess);
		
		// Dibujar objeto
		gl_bindVAO(obj.vao);
		glDrawElements(GL_TRIANGLES, 3 * obj.numTriangles, GL_UNSIGNED_INT, 0);
	}
	
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

// ------------------------------------------------------------------------------------------
// Callback de teclado
// ------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
	
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		load();

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		// Movimiento de luz
        float lightSpeed = 0.5f;
		if (key == GLFW_KEY_I) g_light_dir.y += lightSpeed;
		if (key == GLFW_KEY_K) g_light_dir.y -= lightSpeed;
		if (key == GLFW_KEY_J) g_light_dir.x -= lightSpeed;
		if (key == GLFW_KEY_L) g_light_dir.x += lightSpeed;
		if (key == GLFW_KEY_U) g_light_dir.z += lightSpeed;
		if (key == GLFW_KEY_O) g_light_dir.z -= lightSpeed;
		
		// Activar/desactivar sombras
		if (key == GLFW_KEY_T && action == GLFW_PRESS) {
			g_useShadows = !g_useShadows;
			cout << "Sombras: " << (g_useShadows ? "Activadas" : "Desactivadas") << endl;
		}
		
		// Movimiento de cámara FPS
            float currentSpeed = g_cameraSpeed * g_deltaTime;
            vec3 cameraRight = normalize(cross(g_cameraFront, g_cameraUp));

		if (key == GLFW_KEY_W) g_cameraPos += currentSpeed * g_cameraFront;
		if (key == GLFW_KEY_S) g_cameraPos -= currentSpeed * g_cameraFront;
		if (key == GLFW_KEY_A) g_cameraPos -= currentSpeed * cameraRight;
		if (key == GLFW_KEY_D) g_cameraPos += currentSpeed * cameraRight;
		if (key == GLFW_KEY_SPACE) g_cameraPos += currentSpeed * g_cameraUp;
		if (key == GLFW_KEY_LEFT_SHIFT) g_cameraPos -= currentSpeed * g_cameraUp;
	}
}

// ------------------------------------------------------------------------------------------
// Callback de mouse para cámara FPS
// ------------------------------------------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (g_firstMouse) {
        g_lastX = xpos;
        g_lastY = ypos;
        g_firstMouse = false;
        return;
    }

    float xoffset = xpos - g_lastX;
	float yoffset = g_lastY - ypos;
    g_lastX = xpos;
    g_lastY = ypos;

    xoffset *= g_mouseSensitivity;
    yoffset *= g_mouseSensitivity;

        g_cameraYaw += xoffset;
        g_cameraPitch += yoffset;
        
        if (g_cameraPitch > 89.0f) g_cameraPitch = 89.0f;
        if (g_cameraPitch < -89.0f) g_cameraPitch = -89.0f;
        
        vec3 direction;
        direction.x = cos(radians(g_cameraYaw)) * cos(radians(g_cameraPitch));
        direction.y = sin(radians(g_cameraPitch));
        direction.z = sin(radians(g_cameraYaw)) * cos(radians(g_cameraPitch));
        g_cameraFront = normalize(direction);
}

// ------------------------------------------------------------------------------------------
// Callback de botones de mouse
// ------------------------------------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// No usado actualmente
}

// ------------------------------------------------------------------------------------------
// Función principal
// ------------------------------------------------------------------------------------------
int main(void)
{
	// Inicializar GLFW
	GLFWwindow* window;
	if (!glfwInit()) return -1;
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	window = glfwCreateWindow(g_ViewportWidth, g_ViewportHeight, "OpenGL Scene - Judith Caralt & Isabela Diaz", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	// Configurar callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Configurar OpenGL
	glClearColor(g_backgroundColor.x, g_backgroundColor.y, g_backgroundColor.z, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Cargar recursos
	load();
	
	// Inicializar dirección de cámara
	vec3 direction;
	direction.x = cos(radians(g_cameraYaw)) * cos(radians(g_cameraPitch));
	direction.y = sin(radians(g_cameraPitch));
	direction.z = sin(radians(g_cameraYaw)) * cos(radians(g_cameraPitch));
	g_cameraFront = normalize(direction);

	// Loop principal
	while (!glfwWindowShouldClose(window)) {
		// Calcular deltaTime
		float currentFrame = (float)glfwGetTime();
		g_deltaTime = currentFrame - g_lastFrame;
		g_lastFrame = currentFrame;
		
		// Renderizar
		draw();

		// Intercambiar buffers
        glfwSwapBuffers(window);
        
		// Procesar eventos
        glfwPollEvents();
        
		// Obtener posición del mouse
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
    }

	// Limpiar y terminar
    glfwTerminate();
    return 0;
}
