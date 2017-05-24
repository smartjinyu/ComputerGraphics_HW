attribute vec2 av2texCoord;
attribute vec4 av4position;

uniform mat4 mvp;
uniform mat4 um4modelMatrix;

varying vec2 vv2texCoord;
varying vec4 vv4vertex;

void main()
{
	gl_Position = mvp * av4position;
	
	vv2texCoord = av2texCoord;
	vv4vertex = um4modelMatrix * av4position;
}
