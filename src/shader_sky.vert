#version 330

in vec3 a_vertex;
in vec2 a_texcoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 v_texcoord;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(a_vertex, 1.0);
	v_texcoord = a_texcoord;
}

