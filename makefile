gfx: src/main.cpp src/glfunctions.cpp src/Shader.cpp
	clang++ -std=c++11 -stdlib=libc++ -o gfx -lglfw -lglew -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon src/main.cpp src/glfunctions.cpp src/Shader.cpp
