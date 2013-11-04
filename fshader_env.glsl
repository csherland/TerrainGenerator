varying vec4 color;
varying vec4 texts;
varying vec2 texCoord;

uniform sampler2D waterTex;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D snowTex;

void main() 
{ 	     
	if (texts.x > 0.1) {
		gl_FragColor = color*texture2D(waterTex, texCoord);
	} else if (texts.y > 0.1) {
		gl_FragColor = color*texture2D(grassTex, texCoord);
	} else if (texts.z > 0.1) {
		gl_FragColor = color*texture2D(rockTex, texCoord);
	} else {
		gl_FragColor = color*texture2D(snowTex, texCoord);
	}
} 

