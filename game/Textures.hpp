#pragma once

#include <engine/Graphics/Texture.hpp>

Texture makeFloatTexture(i64 sizeX, i64 sizeY);
void updateFloatTexture(f32* data, i64 sizeX, i64 sizeY);