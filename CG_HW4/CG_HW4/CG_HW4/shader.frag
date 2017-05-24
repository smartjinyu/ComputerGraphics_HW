// TODO: uncomment this line
// uniform sampler2D us2dtexture;
varying vec2 vv2texCoord;
varying vec4 vv4colour, vv4vertex;

void main()
{
	gl_FragColor = vec4(vv4vertex.zzz/2.0/sqrt(3.0)+0.5, 0.0);
	// TODO: deal with the texture color
	// vec4 v4texColor = texture2D(us2dtexture, vv2texCoord).bgra;
}
