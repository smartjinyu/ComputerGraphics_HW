attribute vec4 av4position;
attribute vec3 av3normal;

varying vec4 vv4color;

uniform mat4 mvp;

struct LightSourceParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
	vec4 halfVector;
	vec3 spotDirection;
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

struct MaterialParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform MaterialParameters Material;
uniform LightSourceParameters LightSource[3];

void main() {
	
	vec4 vv4ambient_D = Material.ambient * LightSource[0].ambient;
	vv4color = vv4ambient_D;
	gl_Position = mvp * av4position;
}