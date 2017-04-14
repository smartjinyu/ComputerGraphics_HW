varying vec3 vv3color;

uniform vec3 colorfilter;

void main() {
	gl_FragColor = vec4(vv3color.x*colorfilter.x, vv3color.y*colorfilter.y, vv3color.z*colorfilter.z, 1.0);
}
