#version 430 core

in vec2 fragTexturePosition; 
out vec4 fragColor;

/*generated end*/

uniform sampler2D waveTexture;

#include "scientificColoring.glsl"

void main() {
	fragColor = vec4(scientificColoring(texture(waveTexture, fragTexturePosition).r, 0.0, 1.0), 0.5);
}
