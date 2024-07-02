#version 430 core

in vec2 fragTexturePosition; 
out vec4 fragColor;

/*generated end*/

uniform sampler2D waveTexture;

void main() {
	fragColor = vec4(fragTexturePosition, 0.0, 1.0);
	fragColor = texture(waveTexture, fragTexturePosition);
	//fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
