#include "Textures.hpp"
#include <glad/glad.h>

Texture makeFloatTexture(i64 sizeX, i64 sizeY) {
	auto result = Texture::generate();

	result.bind();
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_R32F,
		sizeX,
		sizeY,
		0,
		GL_RED,
		GL_FLOAT,
		nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return result;
}

void updateFloatTexture(f32* data, i64 sizeX, i64 sizeY) {
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sizeX, sizeY, GL_RED, GL_FLOAT, data);
}
