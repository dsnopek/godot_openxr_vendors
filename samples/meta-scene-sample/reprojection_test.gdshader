shader_type spatial;
//render_mode unshaded, shadows_disabled, cull_disabled, depth_draw_always;
render_mode shadow_to_opacity, shadows_disabled, cull_disabled, depth_draw_always;

global uniform bool META_ENVIRONMENT_DEPTH_AVAILABLE;
global uniform highp sampler2DArray META_ENVIRONMENT_DEPTH_TEXTURE : filter_nearest, repeat_disable, hint_default_black;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT;
global uniform highp mat4 META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT;

void vertex() {
	UV = VERTEX.xy * 0.5 + 0.5;
	POSITION = vec4(VERTEX.xyz, 1.0);
}

void fragment() {
	highp mat4 camera_to_depth_proj = (VIEW_INDEX == VIEW_MONO_LEFT) ? META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT;
	highp mat4 depth_to_camera_proj = (VIEW_INDEX == VIEW_MONO_LEFT) ? META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT;

	highp vec4 clip = vec4(UV * 2.0 - 1.0, 1.0, 1.0);
	highp vec4 reprojected = camera_to_depth_proj * clip;
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
	clip_back = depth_to_camera_proj * clip_back;
	highp float camera_ndc_z = clip_back.z / clip_back.w;
	highp float camera_depth = CLIP_SPACE_FAR < 0.0 ? (camera_ndc_z * 0.5 + 0.5) : camera_ndc_z;

	//ALBEDO = vec3(1.0 - camera_depth);
	ALBEDO = vec3(0.0, 0.0, 0.0);
	DEPTH = 1.0 - camera_depth;
}