#version 330 core
in vec2 texCoords;
out vec4 fragColor;
uniform vec4 color;
uniform sampler2D tex;
uniform bool useColor;
void main(){
	if(useColor) fragColor = color;
	else
	{
		fragColor = texture(tex, texCoords);
	}
}