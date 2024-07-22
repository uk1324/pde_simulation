#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in vec3 vertexColor; 

uniform mat4 transform; 

out vec3 normal; 
out vec3 color; 

/*generated end*/

void main() {
	normal = vertexNormal;
	color = vertexColor;
	gl_Position = transform * vec4(vertexPosition, 1.0);
}
