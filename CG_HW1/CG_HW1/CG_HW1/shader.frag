uniform int fmode;
varying vec3 vv3color;

void main() {
	if(fmode == 0){
		gl_FragColor = vec4(vv3color, 1.0);
		return;
	}

	if(fmode == 1){
		mat3 filter = mat3(
			vec3(1,0,0),
			vec3(0,0,0),
			vec3(0,0,0));
		gl_FragColor = vec4(filter * vv3color, 1.0);
		return;
	}
	if(fmode == 2){
		mat3 filter = mat3(
			vec3(0,0,0),
			vec3(0,1,0),
			vec3(0,0,0));
		gl_FragColor = vec4(filter * vv3color, 1.0);
		return;
	}
	if(fmode == 3){
		mat3 filter = mat3(
			vec3(0,0,0),
			vec3(0,0,0),
			vec3(0,0,1));
		gl_FragColor = vec4(filter * vv3color, 1.0);
		return;
	}
}
