uniform mat4 MVP;

// "Vertex" attribute
attribute vec3 vCol;
attribute vec3 vPos;		// was: vec2 vPos

varying vec3 color;


void main()
{
    //gl_Position = MVP * vec4(vPos, 0.0, 1.0);	
	vec3 position = vPos;
	gl_Position = MVP * vec4(position, 1.0f);
	
    color = vCol;
}
