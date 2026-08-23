#version 330

// Texturas
uniform sampler2D u_texture;          // Textura difusa
uniform sampler2D u_textureNormal;    // Mapa de normales
uniform sampler2D u_textureSpecular; // Mapa especular
uniform sampler2D u_textureAlpha;     // Mapa alfa

// Flags de uso de texturas
uniform int u_useTexture;
uniform int u_useNormalMap;
uniform int u_useSpecularMap;
uniform int u_useAlphaMap;

// Uniforms de iluminación
uniform vec3 u_light_dir;
uniform vec3 u_cam_pos;
uniform vec3 u_ambient;
uniform vec3 u_diffuse;
uniform vec3 u_specular;
uniform float u_shininess;

// Mapeo de sombras
uniform sampler2D u_shadowMap;
uniform int u_useShadows;

out vec4 fragColor;

in vec2 v_texcoord;
in vec3 v_normal;
in vec3 v_vertex;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec4 v_fragPosLightSpace;

// Función para calcular sombras
float ShadowCalculation(vec4 fragPosLightSpace)
{
	if (u_useShadows == 0) return 1.0;
	
	// Perspectiva divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// Transformar a rango [0,1]
	projCoords = projCoords * 0.5 + 0.5;
	
	// Obtener profundidad más cercana desde la perspectiva de la luz
	float closestDepth = texture(u_shadowMap, projCoords.xy).r;
	
	// Obtener profundidad del fragmento actual desde la perspectiva de la luz
	float currentDepth = projCoords.z;
	
	// Verificar si el fragmento está en sombra
	float shadow = currentDepth > closestDepth + 0.005 ? 0.3 : 1.0;
	
	// Mantener sombras solo dentro del rango del frustum de la luz
	if (projCoords.z > 1.0)
		shadow = 1.0;
	
	return shadow;
}

void main(void)
{
	// Obtener color de textura difusa
	vec3 texture_color;
	if (u_useTexture == 1) {
		texture_color = texture(u_texture, v_texcoord).rgb;
	} else {
		texture_color = vec3(0.8, 0.8, 0.8);
	}
	
	// Obtener alpha desde mapa alfa o textura difusa
	float alpha = 1.0;
	if (u_useAlphaMap == 1) {
		alpha = texture(u_textureAlpha, v_texcoord).r;
	} else if (u_useTexture == 1) {
		alpha = texture(u_texture, v_texcoord).a;
	}
	
	// Descartar píxeles completamente transparentes
	if (alpha < 0.01) {
		discard;
	}
	
	// Calcular normal - usar mapa de normales si está disponible
	vec3 N;
	if (u_useNormalMap == 1) {
		// Muestrear normal desde mapa de normales (convertir de [0,1] a [-1,1])
		vec3 normalMapSample = texture(u_textureNormal, v_texcoord).rgb * 2.0 - 1.0;
		
		// Crear matriz TBN (Tangent-Bitangent-Normal)
		mat3 TBN = mat3(normalize(v_tangent), normalize(v_bitangent), normalize(v_normal));
		
		// Transformar normal de espacio tangente a espacio del mundo
		N = normalize(TBN * normalMapSample);
	} else {
		N = normalize(v_normal);
	}
	
	// Calcular dirección de la luz (normalizada)
	vec3 L = normalize(u_light_dir - v_vertex);
	
	// Calcular dirección de vista
	vec3 E = normalize(u_cam_pos - v_vertex);
	
	// Calcular vector mitad para Blinn-Phong
	vec3 H = normalize(L + E);
	
	// Calcular NdotL (limitado a 0)
	float NdotL = max(dot(N, L), 0.0);
	
	// Calcular NdotH para especular Blinn-Phong
	float NdotH = max(dot(N, H), 0.0);
	
	// Obtener intensidad especular desde mapa especular si está disponible
	float specularIntensity = 1.0;
	if (u_useSpecularMap == 1) {
		specularIntensity = texture(u_textureSpecular, v_texcoord).r;
	}
	
	// Calcular sombras
	float shadow = ShadowCalculation(v_fragPosLightSpace);
	
	// Componente ambiente
	vec3 ambient_color = texture_color * u_ambient;
	
	// Componente difusa (modulada por sombras)
	vec3 diffuse_color = texture_color * NdotL * u_diffuse * shadow;
	
	// Componente especular (Blinn-Phong) - modulada por mapa especular y sombras
	vec3 spec_color = texture_color * pow(NdotH, u_shininess) * u_specular * specularIntensity * shadow;
	
	// Color final = ambiente + difusa + especular
	vec3 final_color = ambient_color + diffuse_color + spec_color;
	
	fragColor = vec4(final_color, alpha);
}
