shader_type sky;
render_mode use_debanding;

uniform vec3 coefficients[9];
uniform mat3 rotation;
uniform int sh_l = 2;

#define SH_L 1

vec3 applySH(vec3 dir, vec3 sh[9]) {
	float x = dir.x;
	float y = dir.y;
	float z = dir.z;

	// SH basis constants.
	const float c0 = 0.2820947918;  // 1/(2*sqrt(pi))
	const float c1 = 0.4886025119;  // sqrt(3)/(2*sqrt(pi))
	const float c2 = 1.0925484306;  // sqrt(15)/(2*sqrt(pi))
	const float c3 = 0.3153915653;  // sqrt(5)/(4*sqrt(pi))
	const float c4 = 0.5462742153;  // sqrt(15)/(4*sqrt(pi))

	vec3 result = vec3(0.0);

	// L0
	if (sh_l >= 0) {
		result += sh[0] * c0;
	}

	// L1
	if (sh_l >= 1) {
		result += sh[1] * (c1 * y);
		result += sh[2] * (c1 * z);
		result += sh[3] * (c1 * x);
	}

	// L2
	if (sh_l >= 2) {
		result += sh[4] * (c2 * x * y);
		result += sh[5] * (c2 * y * z);
		result += sh[6] * (c3 * (3.0 * z * z - 1.0));
		result += sh[7] * (c2 * x * z);
		result += sh[8] * (c4 * (x * x - y * y));
	}

	return result;
}

vec3 linear_to_srgb(vec3 color) {
	const vec3 a = vec3(0.055f);
	return mix((vec3(1.0f) + a) * pow(color.rgb, vec3(1.0f / 2.4f)) - a, 12.92f * color.rgb, lessThan(color.rgb, vec3(0.0031308f)));
}

void sky() {
	if (AT_CUBEMAP_PASS) {
		vec3 dir = rotation * EYEDIR;
		vec3 luminance = applySH(dir, coefficients);
		luminance = max(luminance, vec3(0.0));
#if CURRENT_RENDERER == RENDERER_COMPATIBILITY
		COLOR = linear_to_srgb(luminance);
#else
		COLOR = luminance;
#endif
	} else {
		// Allows visualizing the radiance map.
		COLOR = texture(RADIANCE, EYEDIR).rgb;
	}
}
