#version 430 core
out vec4 fragColor;

/*generated end*/

#include "scientificColoring.glsl"
in float height;
in vec3 normal;

void main() {
	fragColor = vec4(scientificColoring(height, 0.0, 1.0) * clamp((normalize(normal).y + 0.8), 0.0, 1.0), 1.0);
}
