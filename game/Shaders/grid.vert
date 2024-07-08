#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 
layout(location = 2) in mat3x2 instanceClipToWorld; 
layout(location = 5) in float instanceCameraZoom; 
layout(location = 6) in float instanceSmallCellSize; 

out vec2 worldPosition; 

out mat3x2 clipToWorld; 
out float cameraZoom; 
out float smallCellSize; 

void passToFragment() {
    clipToWorld = instanceClipToWorld; 
    cameraZoom = instanceCameraZoom; 
    smallCellSize = instanceSmallCellSize; 
}

/*generated end*/

void main() {
	passToFragment();
	worldPosition = (vertexTexturePosition - vec2(0.5)) * 2.0;
	worldPosition = clipToWorld * vec3(worldPosition, 1.0);
	gl_Position = vec4(vertexPosition, 0.0, 1.0);
}