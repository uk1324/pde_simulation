#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 
layout(location = 2) in mat3x2 instanceTransform; 

out vec2 fragTexturePosition; 

/*generated end*/

void main() {
	gl_Position = vec4(instanceTransform * vec3(vertexPosition, 1.0), 0.0, 1.0);
	fragTexturePosition = vertexTexturePosition;
}
