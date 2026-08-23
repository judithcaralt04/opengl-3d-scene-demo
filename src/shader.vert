#version 330
 
in vec3 a_vertex;
in vec2 a_texcoord;
in vec3 a_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_normalMatrix;
uniform mat4 u_lightSpaceMatrix;

out vec2 v_texcoord;
out vec3 v_normal;
out vec3 v_vertex;
out vec3 v_tangent;
out vec3 v_bitangent;
out vec4 v_fragPosLightSpace;

void main()
{
	// Posición del vértice
    gl_Position = u_projection * u_view * u_model * vec4( a_vertex , 1.0 );

	// Pasar las coordenadas de textura al fragment shader
	v_texcoord = a_texcoord;
	
	// Transformar normal usando matriz normal
	v_normal = normalize((u_normalMatrix * vec4(a_normal, 0.0)).xyz);
	
	// Pasar posición del vértice en espacio del mundo
	v_vertex = (u_model * vec4(a_vertex, 1.0)).xyz;
	
	// Calcular tangente y bitangente para mapeo de normales
	// Por simplicidad, calculamos un espacio tangente básico
	// En una implementación completa, estos vendrían del mesh
	vec3 N = normalize((u_normalMatrix * vec4(a_normal, 0.0)).xyz);
	vec3 tempTangent = vec3(1.0, 0.0, 0.0);
	if (abs(dot(N, tempTangent)) > 0.9) {
		tempTangent = vec3(0.0, 1.0, 0.0);
	}
	v_tangent = normalize(cross(N, tempTangent));
	v_bitangent = normalize(cross(N, v_tangent));
	
	// Calcular posición en espacio de luz para mapeo de sombras
	v_fragPosLightSpace = u_lightSpaceMatrix * u_model * vec4(a_vertex, 1.0);
}

