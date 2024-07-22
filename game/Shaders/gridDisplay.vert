#version 430 core

layout(location = 0) in vec2 vertexPos; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in vec3 instanceScale; 

/*generated end*/

uniform sampler2D heightMap;

out float height;
out vec3 normal;

float normalSample(vec2 p) {
	return texture(heightMap, p / instanceScale.xz).r * instanceScale.y;
}

vec3 getNormal(vec2 p)  {
	vec2 pos = p;

    const vec2 h = vec2(0.01, 0);
    return normalize(vec3(
		normalSample(pos - h.xy) - normalSample(pos + h.xy),
		2.0 * h.x * instanceScale.y,
		normalSample(pos - h.yx) - normalSample(pos + h.yx)
	)); 
}

void main() {
	float y = texture(heightMap, vertexPos).r;	
	height = y;
	normal = getNormal(vertexPos * instanceScale.xz);
	gl_Position = instanceTransform * vec4(vertexPos.x, y, vertexPos.y, 1.0);
}
