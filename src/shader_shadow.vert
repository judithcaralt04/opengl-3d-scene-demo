#version 330

in vec3 a_vertex;

uniform mat4 u_model;
uniform mat4 u_lightSpaceMatrix;

void main()
{
	gl_Position = u_lightSpaceMatrix * u_model * vec4(a_vertex, 1.0);
}

