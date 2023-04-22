//! #include "shared.vert"

uniform vec3 u_diffuse;

out vec3 vertexColor;

vec3 CalculateLighting(vec3 color, vec3 position)
{
	vec3 lightColor = vec3(1);

	// calculate the shading
	// ambient
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;
	
	// diffuse 
	vec3 norm = a_normal;
	vec3 lightDir = u_directionallight.Direction;
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(u_cameraposition - position);
	vec3 reflectDir = reflect(-lightDir, norm);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;  

	vec3 result = (ambient + diffuse + specular) * color;
	return result;
}

void main()
{
	// first transform the position into world space
	vec3 worldSpacePosition = TranslatePosition(a_position);

	// then calucate the lighting
	vertexColor = CalculateLighting(u_diffuse, worldSpacePosition);
	
	ReturnPosition(worldSpacePosition);
}