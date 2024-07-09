#version 430 core

in vec2 worldPosition; 

in mat3x2 clipToWorld; 
in float cameraZoom; 
in float smallCellSize; 
out vec4 fragColor;

/*generated end*/

void main() {
	float halfSmallCellSize = smallCellSize / 2.0;
	vec2 worldPos = worldPosition;
	// Translate so every cell contains a cross of lines instead of a square. When a square is used it contains 4 lines which makes coloring specific lines difficult. Centering it makes it so there are only 2 lines in each cell.
	worldPos += vec2(halfSmallCellSize, halfSmallCellSize);
	// Translate [0, 0] to center of cell.
	vec2 posInCell = mod(abs(worldPos), smallCellSize) - vec2(halfSmallCellSize, halfSmallCellSize); 

	float dVertical = abs(dot(posInCell, vec2(1.0, 0.0)));
	float dHorizontal = abs(dot(posInCell, vec2(0.0, 1.0)));
	float width = 0.0015 / cameraZoom;
	width += 0.005;
	float interpolationWidth = width / 5.0;
	// Flip colors by making the second argument to smoothstep smaller than the first one.
	dVertical = smoothstep(width, width - interpolationWidth, dVertical);
	dHorizontal = smoothstep(width, width - interpolationWidth, dHorizontal);

	vec3 col0 = vec3(0.12, 0.12, 0.12);
	vec3 col1 = col0 / 2.0;
	ivec2 cellPos = ivec2(floor(worldPos / smallCellSize));
	vec3 colVertical = (cellPos.x % 4 == 0) ? col0 : col1;
	vec3 colHorizontal = (cellPos.y % 4 == 0) ? col0 : col1;
	colVertical *= dVertical;
	colHorizontal *= dHorizontal;

//	vec3 gridColor = max(colVertical, colHorizontal);
//	vec3 backgroundColor = vec3(0.294, 0.545, 0.784);
//
//	fragColor = vec4(mix(backgroundColor, gridColor, max(dVertical, dHorizontal)), 1);
	fragColor = vec4(max(colVertical, colHorizontal), 1);
}