varying vec3 color;

uniform vec4 diffuseColour; 	// New variable (uniform makes it exposed)

void main()
{	
	// Built-in GLSL variable
	// It's the colour of the pixel (fragment)
//    gl_FragColor = vec4(color.r, color.g, color.b, 1.0);

	gl_FragColor = diffuseColour;
}

