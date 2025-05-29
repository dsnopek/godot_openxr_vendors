shader_type spatial;
render_mode unshaded, shadows_disabled, cull_disabled, depth_draw_always;
//render_mode shadow_to_opacity, shadows_disabled, cull_disabled, depth_draw_always;

global uniform bool META_ENVIRONMENT_DEPTH_AVAILABLE;
global uniform highp sampler2DArray META_ENVIRONMENT_DEPTH_TEXTURE : filter_nearest, repeat_disable, hint_default_black;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT;

varying vec2 uv_interp;

void vertex() {
	uv_interp = VERTEX.xy * 0.5 + 0.5;
	POSITION = vec4(VERTEX.xyz, 1.0);
}

void fragment() {
	highp mat4 depth_proj = uint(VIEW_INDEX) == uint(0) ? META_ENVIRONMENT_DEPTH_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT;
	highp mat4 depth_inv_proj = uint(VIEW_INDEX) == uint(0) ? META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT;

	/*
	highp mat4 eye_transform = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(EYE_OFFSET.x, EYE_OFFSET.y, EYE_OFFSET.z, 1.0));

	highp mat4 eye_inv_transform = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(-EYE_OFFSET.x, -EYE_OFFSET.y, -EYE_OFFSET.z, 1.0));
	*/
	highp mat4 eye_transform = mat4(
		vec4(1.0, 0.0, 0.0, EYE_OFFSET.x),
		vec4(0.0, 1.0, 0.0, EYE_OFFSET.y),
		vec4(0.0, 0.0, 1.0, EYE_OFFSET.z),
		vec4(0.0, 0.0, 0.0, 1.0));

	highp mat4 eye_inv_transform = mat4(
		vec4(1.0, 0.0, 0.0, -EYE_OFFSET.x),
		vec4(0.0, 1.0, 0.0, -EYE_OFFSET.y),
		vec4(0.0, 0.0, 1.0, -EYE_OFFSET.z),
		vec4(0.0, 0.0, 0.0, 1.0));

	highp vec4 clip = vec4(uv_interp * 2.0 - 1.0, 1.0, 1.0);
	highp vec4 world_pos = INV_VIEW_MATRIX * eye_inv_transform * INV_PROJECTION_MATRIX * clip;
	world_pos /= world_pos.w;

	highp vec4 reprojected = depth_proj * world_pos;
	reprojected /= reprojected.w;
	highp vec2 reprojected_uv = reprojected.xy * 0.5 + 0.5;

	highp float depth = 0.0;
	if (reprojected_uv.x >= 0.0 && reprojected_uv.y >= 0.0 && reprojected_uv.x <= 1.0 && reprojected_uv.y <= 1.0) {
		depth = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(reprojected_uv, float(VIEW_INDEX))).r;
	}
	if (depth == 0.0) {
		discard;
	}

	highp vec4 clip_back = vec4(reprojected.xy, depth * 2.0 - 1.0, 1.0);
	highp vec4 world_back = depth_inv_proj * clip_back;
	world_back /= world_back.w;

	highp vec4 camera_clip = PROJECTION_MATRIX * eye_transform * VIEW_MATRIX * world_back;
	highp float camera_ndc_z = camera_clip.z / camera_clip.w;
	highp float camera_depth = camera_ndc_z * 0.5 + 0.5;

	ALBEDO = vec3(camera_depth);
	//ALBEDO = vec3(0.0, 0.0, 0.0);
	DEPTH = camera_depth;
}
