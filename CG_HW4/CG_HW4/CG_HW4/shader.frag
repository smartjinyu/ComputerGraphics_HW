#version 110
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
	int lighttype;
	int enable;
};

struct MaterialParameters {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform mat4 mvp;
uniform mat4 um4rotateMatrix, um4modelMatrix;
uniform MaterialParameters Material;
uniform LightSourceParameters LightSource[3];
uniform int LIghtcount;
uniform mat4 View;
uniform mat4 Model;
uniform mat4 Proj;
uniform vec3 eyepos;
varying vec4 vv4ambient, vv4diffuse;
varying vec3 vv3normal, vv3halfVector;
varying vec2 v2texCoord;
varying vec4 vv4color, vv4vertice;
uniform int PV_FLAG;


void main() {
	vec4 Diffuse[3], Ambient[3];
	vec3 halfVector[3];
	vec4 color = vec4(0.0,0.0,0.0,0.0);
    vec3 viewVec = normalize(eyepos - vv4vertice.xyz);
	float NdotL, NdotHV;
	float spotEffect;
	vec3 n = normalize(vv3normal);
	vec3 halfV;
	for(int i =0;i<3;i++)
	{
		LightSourceParameters LightSource_now = LightSource[i];
		
		vec3 lightDir;
		float att,dist;
		
		Diffuse[i] = Material.diffuse * LightSource_now.diffuse;
		Ambient[i] = Material.ambient * LightSource_now.ambient;
		color += Ambient[i];
		vec3 lightVec = normalize( LightSource_now.position.xyz)- normalize(vv4vertice.xyz);
		halfVector[i] = normalize(lightVec + viewVec);
		if(LightSource_now.lighttype==0 && LightSource_now.enable == 1)	//Direction Light;
		{
		  att = 1.0; 
		  lightDir = normalize(vec3(LightSource_now.position));		 
		  NdotL = max(dot(n,lightDir),0.0);
			if (NdotL > 0.0) {
				color += Diffuse[i] * NdotL;
				halfV = normalize(halfVector[i]);
				NdotHV = abs(dot(n,halfV));
				color += Material.specular *
						LightSource_now.specular *
						pow(NdotHV, Material.shininess);	
			}
		}
		
		else if(LightSource_now.lighttype==1 && LightSource_now.enable == 1) // Positional Light
		{
			lightDir = vec3(LightSource_now.position-vv4vertice);
			dist = length(lightDir);		
			lightDir = normalize(lightDir);
			NdotL = max(dot(n,lightDir),0.0);
			
			if (NdotL > 0.0) {
		 
			att = 1.0 / (LightSource_now.constantAttenuation +
					LightSource_now.linearAttenuation * dist +
					LightSource_now.quadraticAttenuation * dist * dist);
			color += att * (Diffuse[i] * NdotL + Ambient[i]);
		 
			 
			halfV = normalize(halfVector[i]);
			NdotHV = abs(dot(n,halfV));
			color += att * Material.specular * LightSource_now.specular * pow(NdotHV,Material.shininess);
			}
			
			
		
		}
		else if(LightSource_now.lighttype==2 && LightSource_now.enable == 1) // Spot Light
		{
			lightDir = vec3(LightSource_now.position-vv4vertice);
			dist = length(lightDir);
			NdotL = max(dot(n,normalize(lightDir)),0.0);
			
			if (NdotL > 0.0) {
			spotEffect =   dot(normalize(LightSource_now.spotDirection), normalize(-lightDir));		
			if (spotEffect > LightSource_now.spotCosCutoff) {
			
				spotEffect = pow(spotEffect, LightSource_now.spotExponent);
				att = spotEffect / (LightSource_now.constantAttenuation +
						LightSource_now.linearAttenuation * dist +
						LightSource_now.quadraticAttenuation * dist * dist);
					 
				color += att * (Diffuse[i] * NdotL + Ambient[i]);
				halfV = normalize(halfVector[i]);
				NdotHV = abs(dot(n,halfV));
				color += att * Material.specular * LightSource_now.specular * pow(NdotHV,Material.shininess);
				
				}
			}
			
		}
	}
	if(PV_FLAG == 0)
		gl_FragColor = color;
	else
		gl_FragColor = vv4color;	
}
