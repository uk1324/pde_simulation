vec3 scientificColoring(float v, float minV, float maxV) {
	v = min(max(v, minV), maxV - 0.01);
	float d = maxV - minV;
	v = d == 0.0 ? 0.5 : (v - minV) / d;
	float m = 0.25;
	int num = int(floor(v / m));
	float s = (v - num * m) / m;

	float r = 0.0, g = 0.0, b = 0.0;

	switch (num) {
	case 0: r = 0.0; g = s; b = 1.0; break;
	case 1: r = 0.0; g = 1.0; b = 1.0 - s; break;
	case 2: r = s; g = 1.0; b = 0.0; break;
	case 3: r = 1.0; g = 1.0 - s; b = 0.0; break;
	}

	if (r == 0.0 && g == 0.0 && b == 0.0) {
		g = 1.0;
	}

	return vec3(r, g, b);
}