# OpenGL 3D Scene

A complete 3D scene built in C++ and OpenGL, showcasing every major technique covered in the course. The scene contains 13 objects with different shapes and textures, Blinn-Phong lighting, a skybox, complex animations, alpha blending, normal and specular mapping, and shadow mapping as the advanced technique.

This is an individual project.

## Tech stack

- C++
- OpenGL, with GLFW for windowing and input, and GLEW for extension loading
- GLM for vector and matrix math
- stb_image for texture loading and tinyobjloader for loading `.obj` models
- Custom vertex and fragment shaders, including separate shaders for shadow mapping and the skybox

## How to build and run

Before building, unzip `include.zip` so that `include/GL`, `include/GLFW` and `include/glm` sit directly inside the project root.

1. Open `visual_studio/Graphics_1.sln` in Visual Studio 2019 or later
2. Build: press `F7`, or Build > Build Solution,
