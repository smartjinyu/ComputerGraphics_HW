attribute vec4 av4position;
attribute vec3 av3normal;

varying vec4 vv4color;


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
uniform LightSourceParameters LightSource[4];
// 0:ambient light  1:directional light  2:point light  3:spot light

uniform int ambientOn;
uniform int diffuseOn;
uniform int specularOn;
uniform int directionalOn;
uniform int pointOn;
uniform int spotOn;

uniform mat4 mvp;
uniform mat4 NormalTransMatrix;
uniform mat4 ModelTransMatrix;
uniform mat4 ViewTransMatrix;

uniform vec3 eyePos;


vec4 calcDirectionalLight(){
	vec4 color = vec4(0.0,0.0,0.0,0.0);
	vec4 vv4position = ModelTransMatrix * av4position;
	vec3 vv3normal = mat3(transpose(inverse(ModelTransMatrix)))*av3normal;
	vec3 L = normalize(LightSource[1].position.xyz-vv4position.xyz);
	vec3 N = normalize(vv3normal);
	vec3 V = normalize (eyePos- vv4position.xyz);
	if(diffuseOn == 1){
		vec4 diffuse = LightSource[1].diffuse * Material.diffuse * max(dot(L,N),0.0);
		color += diffuse;
	}
	if(specularOn == 1){
		vec3 R = reflect(-L,N);
		float spec = pow(max(dot(V,R),0.0),Material.shininess);
		color += Material.specular * LightSource[1].specular  * spec;
	}
	return color;
}

void main() {
	vv4color = vec4(0.0,0.0,0.0,0.0);
	
	if(ambientOn == 1){
		vec4 vv4ambient_D = Material.ambient * LightSource[0].ambient;
		vv4color += vv4ambient_D;
	}
	if(directionalOn == 1){
		vv4color += calcDirectionalLight();
	}

	gl_Position = mvp * av4position;
}