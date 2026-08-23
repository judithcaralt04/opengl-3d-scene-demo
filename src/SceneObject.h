#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

using namespace glm;

/**
 * Estructura para representar un objeto en la escena
 * Contiene toda la información necesaria para renderizar un objeto 3D
 */
struct SceneObject {
    // Identificadores OpenGL
    GLuint vao;                    // Vertex Array Object (Objeto de Array de Vértices)
    GLuint numTriangles;           // Número de triángulos del modelo
    
    // Texturas
    GLuint textureDiffuse;        // Textura difusa (color base)
    GLuint textureNormal;          // Mapa de normales
    GLuint textureSpecular;        // Mapa especular
    GLuint textureAlpha;           // Mapa alfa (para transparencia)
    
    // Propiedades de textura
    bool hasNormalMap;             // Indica si tiene mapa de normales
    bool hasSpecularMap;           // Indica si tiene mapa especular
    bool hasAlphaMap;              // Indica si tiene mapa alfa
    bool isTransparent;            // Indica si es semitransparente
    
    // Transformaciones
    vec3 position;                 // Posición en el mundo
    vec3 scale;                    // Escala en cada eje
    float rotationX, rotationY, rotationZ; // Rotaciones en radianes
    
    // Propiedades de animación
    vec3 basePosition;             // Posición base para animaciones
    float animationTime;           // Tiempo de animación acumulado
    float animationSpeed;          // Velocidad de animación
    int animationType;             // Tipo de animación (0=ninguna, 1=órbita, 2=flotación, 3=espiral, etc.)
    vec3 animationCenter;         // Centro de animación (para órbitas)
    float animationRadius;         // Radio de animación
    
    // Propiedades de iluminación
    float shininess;               // Brillo del material
    vec3 ambientColor;             // Color ambiente
    vec3 diffuseColor;             // Color difuso
    vec3 specularColor;            // Color especular
    
    // Constructor por defecto
    SceneObject() : 
        vao(0), numTriangles(0),
        textureDiffuse(0), textureNormal(0), textureSpecular(0), textureAlpha(0),
        hasNormalMap(false), hasSpecularMap(false), hasAlphaMap(false), isTransparent(false),
        position(0.0f), scale(1.0f), rotationX(0.0f), rotationY(0.0f), rotationZ(0.0f),
        basePosition(0.0f), animationTime(0.0f), animationSpeed(1.0f), animationType(0),
        animationCenter(0.0f), animationRadius(1.0f),
        shininess(32.0f), ambientColor(0.1f), diffuseColor(1.0f), specularColor(1.0f) {}
};

