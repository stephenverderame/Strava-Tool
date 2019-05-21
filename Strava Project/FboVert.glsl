#version 330 core
layout(location = 0) in vec3 pos;
out vec2 texCoords;
uniform mat4 projection;
void main(){
	gl_Position = projection * vec4(pos.xz, 0, 1.0);
	texCoords = normalize(pos.xz);
}