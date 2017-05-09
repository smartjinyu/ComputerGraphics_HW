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
	float spotCutoff; // (range: [0.0f,90.0f], 180.0f)
	float spotCosCutoff; // (range: [1.0f,0.0f],-1.0f)
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

uniform int perPixelOn;

uniform mat4 mvp;
uniform mat4 NormalTransMatrix;
uniform mat4 ModelTransMatrix;
uniform mat4 ViewTransMatrix;

uniform vec3 eyePos;

vec3 vv3normal = mat3(transpose(inverse(ModelTransMatrix)))*av3normal;

varying vec4 vv4position;
varying vec3 N;
varying vec3 V;

vec4 calcDirectionalLight(LightSourceParameters lightSource){
	vec4 color = vec4(0.0f,0.0f,0.0f,0.0f);
	vec3 L = normalize(lightSource.position.xyz-vv4position.xyz);
	if(diffuseOn == 1){
		vec4 diffuse = lightSource.diffuse * Material.diffuse * max(dot(L,N),0.0f);
		diffuse = clamp(diffuse,0.0f,1.0f);
		color += diffuse;
	}
	if(specularOn == 1){
		vec3 R = normalize(reflect(-L,N));
		float spec = pow(max(dot(V,R),0.0f),65.0f);
		vec4 specular = Material.specular * lightSource.specular * spec;
		specular = clamp(specular,0.0f,1.0f);
		color += specular;
	}
	return color;
}

vec4 calcPointLight(LightSourceParameters lightSource){
	vec4 color = vec4(0.0f,0.0f,0.0f,0.0f);
	float distance = length(lightSource.position.xyz-vv4position.xyz);
	float attenuation = 1.0f/(lightSource.constantAttenuation 
				+ lightSource.linearAttenuation * distance 
				+ lightSource.quadraticAttenuation * distance * distance);
	vec3 L = normalize(lightSource.position.xyz-vv4position.xyz);
	if(diffuseOn == 1){
		vec4 diffuse = lightSource.diffuse * Material.diffuse * max(dot(L,N),0.0f);
		diffuse *= attenuation;
		diffuse = clamp(diffuse,0.0f,1.0f);
		color += diffuse;
	}
	if(specularOn == 1){
		vec3 R = normalize(reflect(-L,N));
		float spec = pow(max(dot(V,R),0.0f),65.0f);
		vec4 specular = Material.specular * lightSource.specular * spec;
		specular *= attenuation;
		specular = clamp(specular,0.0f,1.0f);
		color += specular;
	}

	return color;
}

vec4 calcSpotLight(LightSourceParameters lightSource){
	vec4 color = vec4(0.0f,0.0f,0.0f,0.0f);
	float distance = length(lightSource.position.xyz-vv4position.xyz);
	vec3 L = normalize(lightSource.position.xyz-vv4position.xyz);
	float theta = dot(L,normalize(-lightSource.spotDirection));
	float effect = pow(max(dot(L,-lightSource.spotDirection),0.0f),lightSource.spotExponent);
	float attenuation = 1.0f/(lightSource.constantAttenuation 
				+ lightSource.linearAttenuation * distance 
				+ lightSource.quadraticAttenuation * distance * distance);
	if(spotOn == 2 || spotOn == 3){
		lightSource.spotCosCutoff = 0.0f;
		// 2: directional light effect
		// 3: point light effect
	}
	if(theta >= lightSource.spotCosCutoff){
		if(diffuseOn == 1){
			vec4 diffuse = lightSource.diffuse * Material.diffuse * max(dot(L,N),0.0f);
			diffuse *= effect;
			if(spotOn == 3){
				diffuse *= attenuation;
			}
			diffuse = clamp(diffuse,0.0f,1.0f);
			color += diffuse;
		}
		if(specularOn == 1){
			vec3 R = normalize(reflect(-L,N));
			float spec = pow(max(dot(V,R),0.0f),20.0f);
			vec4 specular = Material.specular * lightSource.specular * spec;
			specular *= effect;
			if(spotOn == 3){
				specular *= attenuation;
			}
			specular = clamp(specular,0.0f,1.0f);
			color += specular;

		}
	}
	return color;
}


void main() {
	vv4color = vec4(0.0f,0.0f,0.0f,0.0f);
	vv4position = ModelTransMatrix * av4position;
	N = normalize(vv3normal);
	V = normalize(eyePos- vv4position.xyz);
	gl_Position = mvp * av4position;

	if(perPixelOn == 0){
		// vertex lighting
		if(ambientOn == 1){
			vec4 vv4ambient_D = Material.ambient * LightSource[0].ambient;
			vv4color += vv4ambient_D;
		}
		if(directionalOn == 1){
			vv4color += calcDirectionalLight(LightSource[1]);
		}
		if(pointOn == 1){
			vv4color += calcPointLight(LightSource[2]);
		}
		if(spotOn != 0){
			vv4color += calcSpotLight(LightSource[3]);
		}
	}


} 