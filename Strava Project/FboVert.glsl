#version 330 core
layout(location = 0) in vec3 pos;
out vec2 texCoords;
uniform mat4 projection;
uniform bool useModel;
uniform mat4 model;
void main(){
	if(useModel) gl_Position = projection * model * vec4(pos.xz, 0, 1.0);
	else gl_Position = projection * vec4(pos.xz, 0, 1.0);
	texCoords = pos.xz;
}