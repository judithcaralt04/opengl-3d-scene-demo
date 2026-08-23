Práctica Final - Judith Caralt

Este proyecto implementa una escena virtual 3D completa utilizando OpenGL que demuestra todos los conceptos enseñados en clase. La escena incluye 13 objetos 3D con diferentes formas, texturas, iluminación Blinn-Phong, skybox, animaciones complejas, mezcla alfa, mapas de normales y especulares, y mapeo de sombras (shadow mapping) como técnica avanzada.


Cómo Compilar y Ejecutar


1. Abre `visual_studio/Graphics_1.sln` en Visual Studio 2019 o superior

3. Compilar:
   - Presiona `F7` o selecciona Build > Build Solution
   - Espera a que la compilación termine sin errores

4. Ejecutar:
   - Presiona `F5` para ejecutar en modo debug
   - O presiona `Ctrl+F5` para ejecutar sin debug
   - El ejecutable se generará en `visual_studio/x64/Debug/Graphics
1.exe` o `visual_studio/x64/Release/Graphics_1.exe`

5. Verificar archivos DLL:
   - Asegúrate de que `glew32.dll` y `glfw3.dll` estén en la misma carpeta que el ejecutable
   - Estos archivos están en la raíz de `Graphics_1/` y deben copiarse junto al `.exe` si ejecutas desde otra ubicación



Notas Importantes:
- El programa busca los archivos en la carpeta `assets/` relativa al directorio de trabajo
- Si ejecutas el programa desde Visual Studio, el directorio de trabajo es `Graphics_1/`
- Si ejecutas el `.exe` directamente, asegúrate de hacerlo desde `Graphics_1/` o copia la carpeta `assets/` junto al ejecutable


Cómo Usar el Programa

- Controles de Cámara (FPS - First Person Shooter)

La cámara funciona como en un juego de primera persona:

- W: Mover hacia adelante
- S: Mover hacia atrás
- A: Mover hacia la izquierda 
- D: Mover hacia la derecha 
- Espacio: Subir verticalmente
- Shift Izquierdo: Bajar verticalmente
- Mouse (Arrastrar): Rotar la cámara
  - Mover el mouse horizontalmente: girar izquierda/derecha 
  - Mover el mouse verticalmente: mirar arriba/abajo 

Nota: El cursor del mouse está deshabilitado automáticamente para un control más fluido. La cámara se mueve de forma independiente de la velocidad de frames (usando deltaTime).


Controles de Iluminación

Puedes mover la fuente de luz para ver cómo afecta a los objetos:

- I: Mover la luz hacia arriba
- K: Mover la luz hacia abajo
- J: Mover la luz hacia la izquierda
- L: Mover la luz hacia la derecha
- U: Mover la luz hacia adelante (acercar)
- O: Mover la luz hacia atrás (alejar)


Controles Especiales

- T: Activar/desactivar mapeo de sombras (shadow mapping)
  - Presiona 'T' para ver la diferencia con y sin sombras
  - Las sombras se renderizan desde la perspectiva de la luz
- R: Recargar la escena (recarga todos los recursos)


Consejos de Uso

- Explorar la escena: Usa WASD y el mouse para volar por la escena y observar los objetos desde diferentes ángulos
- Observar animaciones: Los objetos tienen diferentes tipos de animación (órbitas, espirales, péndulos, etc.)
- Experimentar con la luz: Mueve la luz para ver cómo cambia la iluminación en los objetos
- Ver sombras: Activa/desactiva las sombras con 'T' para apreciar el efecto del shadow mapping
- Objetos transparentes: Busca los objetos semitransparentes (teapots) que usan alpha blending

