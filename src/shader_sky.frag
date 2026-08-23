#version 330

uniform sampler2D u_texture;

out vec4 fragColor;

in vec2 v_texcoord;

void main(void)
{
	fragColor = texture(u_texture, v_texcoord);
}

